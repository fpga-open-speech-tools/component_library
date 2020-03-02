----------------------------------------------------------------------------
--! @file FE_CPLD_BME280_I2C_Reader.vhd
--! @brief CPLD BME280 I2C reader component
--! @details This component reads temperature, humidity, pressure from the BME280 via i2c
--! @author Dylan Wickham
--! @date 2020
--! @copyright Copyright 2020 Flat Earth Inc
--
--  Permission is hereby granted, free of charge, to any person obtaining a copy
--  of this software and associated documentation files (the "Software"), to deal
--  IN the Software without restriction, including without limitation the rights
--  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
--  copies of the Software, and to permit persons to whom the Software is furnished
--  to do so, subject to the following conditions:
--
--  The above copyright notice and this permission notice shall be included IN all
--  copies or substantial portions of the Software.
--
--  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
--  INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
--  PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
--  HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
--  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
--  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
--
-- Dylan Wickham
-- Flat Earth Inc
-- 985 Technology Blvd
-- Bozeman, MT 59718
-- support@flatearthinc.com
----------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;
library work;
use work.FE_CPLD_BME280_Compensate_Data.all;

entity FE_CPLD_BME280_I2C_Reader is 
generic ( 
    sdo                   : std_logic := '0';      --0 if SDO is connected to ground, 1 if connected to V_DDIO
                                                   -- sets the LSB of the 7-bit i2c address
    reads_per_second      : integer := 16 ;        --reads per second from BME280
    input_clk             : INTEGER := 50_000_000; --input clock speed from user logic in Hz
    bus_clk               : INTEGER := 400_000);   --speed the i2c bus (scl) will run at in Hz
  
  port (
    sys_clk               : in  std_logic                     := 'X';
    reset_n               : in  std_logic                     := 'X';
    continuous            : in  std_logic                     := 'X'; -- 1 for continuous feed of data at given rate, 0 for a single read
    enable                : in  std_logic                     := 'X'; --starts the process when enabled
    busy_out              : out std_logic := '0';
        
    bme_output_data       : out std_logic_vector(63 downto 0) := (others => '0');
    bme_output_valid      : out std_logic := '0';
    bme_output_error      : out std_logic_vector(1 downto 0) := (others => '0');
    sda                   : INOUT  std_logic;                    --serial data output of i2c bus
    scl                   : INOUT  std_logic);                   --serial clock output of i2c bus

end entity FE_CPLD_BME280_I2C_Reader;

architecture FE_CPLD_BME280_I2C_Reader_arch of FE_CPLD_BME280_I2C_Reader is
CONSTANT divider  :  INTEGER := input_clk/reads_per_second;
component i2c_master IS
  GENERIC(
    input_clk : INTEGER := FE_CPLD_BME280_I2C_Reader.input_clk;  --input clock speed from user logic in Hz
    bus_clk   : INTEGER := FE_CPLD_BME280_I2C_Reader.bus_clk);   --speed the i2c bus (scl) will run at in Hz
  PORT(
    clk       : IN     STD_LOGIC;                    --system clock
    reset_n   : IN     STD_LOGIC;                    --active low reset
    ena       : IN     STD_LOGIC;                    --latch in command
    addr      : IN     STD_LOGIC_VECTOR(6 DOWNTO 0); --address of target slave
    rw        : IN     STD_LOGIC;                    --'0' is write, '1' is read
    data_wr   : IN     STD_LOGIC_VECTOR(7 DOWNTO 0); --data to write to slave
    busy      : OUT    STD_LOGIC;                    --indicates transaction in progress
    data_rd   : OUT    STD_LOGIC_VECTOR(7 DOWNTO 0); --data read from slave
    ack_error : BUFFER STD_LOGIC;                    --flag if improper acknowledge from slave
    sda       : INOUT  STD_LOGIC;                    --serial data output of i2c bus
    scl       : INOUT  STD_LOGIC);                   --serial clock output of i2c bus
END component;

-- BME related constants
constant BME320_I2C_ADDR : std_logic_vector(6 downto 0) := "111011" & sdo;
constant START_ADDR       : std_logic_vector(7 downto 0) := x"F7"; -- memory address to start reading from on BME320
constant BME320_num_calibration : integer := 33;

type i2c_rec is record
  ena  : std_logic;
  addr : std_logic_vector(6 downto 0);
  rw   : std_logic;
  data_wr :std_logic_vector(7 downto 0);
end record i2c_rec;

signal read_i2c  : i2c_rec := (ena => '1', addr => BME320_I2C_ADDR, rw => '1', data_wr => (others => '0'));
signal write_i2c : i2c_rec := (ena => '1', addr => BME320_I2C_ADDR, rw => '0', data_wr => START_ADDR);
signal read_comp_i2c : i2c_rec;
signal i2c_control : i2c_rec;

-- TODO: Set err, etc
-- Create states for the output state machine
type reader_state is (  idle, write, read, process_data, waiting, read_compensation_data);
signal last_reader_state : reader_state := idle;
signal cur_reader_state : reader_state := idle;
signal nex_reader_state : reader_state := write;
signal write_complete   : std_logic := '0';
signal read_complete   : std_logic := '0';
signal process_data_complete   : std_logic := '0';
signal wait_complete   : boolean := false;
signal compensation_data_received :boolean := false;

signal read_data       : std_logic_vector(63 downto 0) := (others => '0');


-- Data byte width definitions
constant temp_byte_width          : integer := 3;
constant humid_byte_width         : integer := 2;
constant pressure_byte_width      : integer := 3;
constant calibration_byte_width   : integer := 2;

-- BME word division definitions
constant temp_byte_location       : integer := 5;
constant humid_byte_location      : integer := 2;
constant pressure_byte_location   : integer := 8;

-- Signals for i2c control
signal i2c_enable : std_logic;
signal i2c_rdwr   : std_logic;
signal i2c_data_write : std_logic(7 downto 0);
signal i2c_busy : std_logic;
signal i2c_data_read : std_logic(7 downto 0);
signal i2c_err : std_logic;

-- Conversion variable used for pressure and humidity, assigned from temperature conversion
signal calibration_err : boolean;

-- TODO: Update for length to use byte width definition
signal temp_actual : signed(31 downto 0);
signal pressure_actual : unsigned(31 downto 0);
signal humid_actual    : unsigned(31 downto 0);

-- Calibration values for temperature, from the BME280
signal dig_t1_std : std_logic_vector(calibration_byte_width * 8 - 1 downto 0);
signal dig_t2_std : std_logic_vector(calibration_byte_width * 8 - 1 downto 0);
signal dig_t3_std : std_logic_vector(calibration_byte_width * 8 - 1 downto 0);

-- Calibration values for pressure, from the BME280
signal dig_p1_std : std_logic_vector(calibration_byte_width * 8 - 1 downto 0);
signal dig_p2_std : std_logic_vector(calibration_byte_width * 8 - 1 downto 0);
signal dig_p3_std : std_logic_vector(calibration_byte_width * 8 - 1 downto 0);
signal dig_p4_std : std_logic_vector(calibration_byte_width * 8 - 1 downto 0);
signal dig_p5_std : std_logic_vector(calibration_byte_width * 8 - 1 downto 0);
signal dig_p6_std : std_logic_vector(calibration_byte_width * 8 - 1 downto 0);
signal dig_p7_std : std_logic_vector(calibration_byte_width * 8 - 1 downto 0);
signal dig_p8_std : std_logic_vector(calibration_byte_width * 8 - 1 downto 0);
signal dig_p9_std : std_logic_vector(calibration_byte_width * 8 - 1 downto 0);

-- Calibrations values for humidity, from the BME280
signal dig_h1_std : std_logic_vector(calibration_byte_width * 8 - 1 downto 0);
signal dig_h2_std : std_logic_vector(calibration_byte_width * 8 - 1 downto 0);
signal dig_h3_std : std_logic_vector(calibration_byte_width * 8 - 1 downto 0);
signal dig_h4_std : std_logic_vector(calibration_byte_width * 8 - 1 downto 0);
signal dig_h5_std : std_logic_vector(calibration_byte_width * 8 - 1 downto 0);
signal dig_h6_std : std_logic_vector(calibration_byte_width * 8 - 1 downto 0);

begin

i2c_component : i2c_master 
generic map (
  input_clk => FE_CPLD_BME280_I2C_Reader.input_clk,  
  bus_clk   => FE_CPLD_BME280_I2C_Reader.bus_clk
)
port map(
  clk       => sys_clk,
  reset_n   => reset_n,
  ena       => i2c_enable,
  addr      => BME320_I2C_ADDR,
  rw        => i2c_rdwr,
  data_wr   => i2c_data_write,
  busy      => i2c_busy,
  data_rd   => i2c_data_read,
  ack_error => i2c_err,
  sda       => FE_CPLD_BME280_I2C_Reader.sda,
  scl       => FE_CPLD_BME280_I2C_Reader.scl
);  

state_management : process (sys_clk, reset_n)
begin
  if reset_n = '0' then 
    cur_reader_state   <= idle;
    next_reader_state  <= write;
    last_reader_state  <= idle;
  else
    last_reader_state <= cur_reader_state;
    case cur_reader_state is 
      when idle =>
        if enable = '1' then
          if compensation_data_received then
            cur_reader_state   <= write;
          else
            cur_reader_state   <= read_compensation_data;
			 end if;
        end if;
      when read_compensation_data =>
        if compensation_data_received then
          cur_reader_state <= write;
        end if;
      when write =>
        if write_complete = '1' then
          cur_reader_state   <= reading;
        end if;
      when read =>
        if read_complete = '1' then
          cur_reader_state   <= process_data;
        end if;
      when process_data =>
        if process_data_complete = '1' then
          if continuous = '1' then
            cur_reader_state <= waiting;
          else
            cur_reader_state <= idle;
          end if;
        end if;
      when waiting =>
        if wait_complete then
          cur_reader_state <= write;
        end if;
      when others => 
    
    end case;
  end if;
end process;

i2c_control : process (sys_clk, reset_n)
  constant i2c_disable : i2c_rec := (ena => '0', addr => (others => '1'), rw => '1', data_wr => (others => '0'));
begin
  case cur_reader_state is 
      when read_compensation_data =>
        i2c_control <= read_comp_i2c;
      when write =>
        i2c_control <= write_i2c;
      when read =>
        i2c_control <= write_i2c;
      when others => 
        i2c_control <= i2c_disable;
    end case;
end process;

write_memory_address : process (sys_clk, reset_n)
  -- when the state just became write
  variable start_write : boolean := (cur_reader_state = write) && (last_reader_state /= write);
begin 
  if start_write then
    --i2c_rdwr <= '0';
    --i2c_data_write <= START_ADDR;
    --i2c_enable <= '1';
  elsif cur_reader_state = write then
    if i2c_busy = '0' then
      write_complete <= '1';
    end if;
  end if;
end process;

read_from_BME320 : process (sys_clk, reset_n)
  -- when the state just became read
  variable start_read : boolean := cur_reader_state = read && last_reader_state /= read;
  variable bytes_read : integer := 0;
  variable read_data_index : integer;
  variable all_bytes_read : boolean := false;
begin
  if start_read then
    -- i2c_rdwr <= '1';
    -- i2c_enable <= 1;
  elsif cur_read_state = read then
    if i2c_busy = '0' then
      read_data_index := read_data'length-8*bytes_read;
      read_data(read_data_index downto (read_data_index - i2c_data_read'length)) <= i2c_data_read;
      bytes_read := bytes_read + 1;

      -- checks if bytes_read equals the number of bytes in read_data
      all_bytes_read := 8 * bytes_read = read_data'length; 
      if(all_bytes_read) then
        read_complete <= '1';
        bytes_read := 0;
      end if;
    end if;
  end if;
end process;

process_data_from_BME320 : process (sys_clk, reset_n)
  variable temp_raw     : std_logic_vector(temp_byte_width * 8 - 1 downto 0);
  variable pressure_raw : std_logic_vector(pressure_byte_width * 8 - 1 downto 0);
  variable humid_raw    : std_logic_vector(humid_byte_width * 8 - 1 downto 0);
begin
  if cur_reader_state = process_data then
    temp_raw := read_data(temp_byte_location * 8 - 1         downto (temp_byte_location - temp_byte_width));
    pressure_raw := read_data(pressure_byte_location * 8 - 1 downto (pressure_byte_location - pressure_byte_width));
    humid_raw := read_data(humid_byte_location * 8 - 1       downto (humid_byte_location - humid_byte_width));

    temp_actual <= TempRawToActual(temp_raw, dig_t1_std, dig_t2_std, dig_t3_std);
    pressure_actual <= PressureRawToActual(pressure_raw, dig_p1_std, dig_p2_std, dig_p3_std, dig_p4_std, dig_p5_std, 
                                           dig_p6_std, dig_p7_std, dig_p8_std, dig_p9_std);
    humid_actual <= HumidRawToActual(humid_raw, dig_h1_std, dig_h2_std, dig_h3_std, dig_h4_std, dig_h5_std, dig_h6_std);                                      
  end if;
end process;

timing : process (sys_clk, reset_n)
    variable counter : integer := 0;
    variable waiting_ended : boolean := false;
begin
  waiting_ended := cur_reader_state /= waiting && last_reader_state = waiting;
  if wait_complete = true then
    -- Do nothing
  elsif counter = divider then
    counter := 0;
    wait_complete := true;
  elsif waiting_ended then
    wait_complete := false;
  else 
    counter := counter + 1;
  end if;

end process;

read_compensation_data_from_BME280 : process (sys_clk, reset_n)
    type compensation_data_reader_state is (idle, init_write, init_read, sec_write, sec_read);
    constant INIT_COMP_ADDR : std_logic_vector(7 downto 0) := x"88";
    constant SEC_COMP_ADDR  : std_logic_vector(7 downto 0) := x"E1";
    constant INIT_BYTES     : integer := 25;
    constant SEC_BYTES      : integer := 8;

    variable read_data      : std_logic_vector((INIT_BYTES + SEC_BYTES) * 8 -1 downto 0);
    variable cur_comp_data_reader_state : compensation_data_reader_state := idle;
    variable init_write_i2c : i2c_rec := (ena => '1', addr => BME320_I2C_ADDR, rw => '0', data_wr => INIT_COMP_ADDR);
    variable sec_write_i2c  : i2c_rec := (ena => '1', addr => BME320_I2C_ADDR, rw => '0', data_wr => SEC_COMP_ADDR);
    -- variable init_read_i2c  : i2c_rec := (ena => '1', addr => BME320_I2C_ADDR, rw => '1', data_wr => (others => '0'));

begin
  if reset_n = '0' then
    cur_comp_data_reader_state <= idle;
    read_comp_i2c <= init_write_i2c;
  else
    case cur_comp_data_reader_state is
      when init_write =>
        read_comp_i2c <= init_write_i2c;
        if i2c_busy = '0' then
          cur_comp_data_reader_state <= init_read;
        end if;
      when init_read =>
        read_comp_i2c <= read_i2c;
        if i2c_busy = '0' then
          read_data_index := read_data'length-8*bytes_read;
          read_data(read_data_index downto (read_data_index - i2c_data_read'length)) <= i2c_data_read;
          bytes_read := bytes_read + 1;
    
          -- checks if bytes_read equals the number of bytes in read_data
          all_bytes_read := bytes_read = INIT_BYTES; 
          if(all_bytes_read) then
            cur_comp_data_reader_state <= sec_write;
          end if;
        end if;
      when sec_write =>
        read_comp_i2c <= sec_write_i2c;
        if i2c_busy = '0' then
          cur_comp_data_reader_state <= sec_read;
        end if;
      when sec_read =>
        read_comp_i2c <= read_i2c;
        if i2c_busy = '0' then
          read_data_index := read_data'length-8*bytes_read;
          read_data(read_data_index downto (read_data_index - i2c_data_read'length)) <= i2c_data_read;
          bytes_read := bytes_read + 1;
    
          -- checks if bytes_read equals the number of bytes in read_data
          all_bytes_read := bytes_read = INIT_BYTES + SEC_BYTES; 
          if(all_bytes_read) then
            cur_comp_data_reader_state <= idle;
            compensation_data_received <= true;
            bytes_read := 0;

            -- TODO: Handle assignments here (w/ procedure?)

          end if;
        end if;
      when idle =>
        -- Do nothing
      when others =>
    end case;
  end if;
     
end process;

end architecture FE_CPLD_BME280_I2C_Reader_arch;