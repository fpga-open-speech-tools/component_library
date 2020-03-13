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
use work.i2c.all;

entity FE_CPLD_BME280_I2C_Reader is 
generic ( 
    sdo                   : std_logic := '0';      --0 if SDO is connected to ground, 1 if connected to V_DDIO
                                                   -- sets the LSB of the 7-bit i2c address
    reads_per_second      : integer := 16 ;        --reads per second from BME280
    input_clk             : INTEGER := 50_000_000); --input clock speed from user logic in Hz
  
  port (
    sys_clk               : in  std_logic                     := 'X';
    reset_n               : in  std_logic                     := 'X';
    continuous            : in  std_logic                     := 'X'; -- 1 for continuous feed of data at given rate, 0 for a single read
    enable                : in  std_logic                     := 'X'; --starts the process when enabled
    busy_out              : out std_logic := '0';
        
    bme_output_data       : out std_logic_vector(95 downto 0) := (others => '0');  -- 3 signed 32 bit words
    bme_output_valid      : out std_logic := '0';
    bme_output_error      : out std_logic_vector(1 downto 0) := (others => '0');
    i2c_ena               : out std_logic;
    i2c_addr              : out std_logic_vector(6 downto 0);
    i2c_rw                : out std_logic;
    i2c_data_wr           : out std_logic_vector(7 downto 0);
    i2c_busy              : in std_logic;
    i2c_data_rd           : in std_logic_vector(7 downto 0);
    i2c_ack_error         : in std_logic);                   

end entity FE_CPLD_BME280_I2C_Reader;

architecture FE_CPLD_BME280_I2C_Reader_arch of FE_CPLD_BME280_I2C_Reader is
CONSTANT divider  :  INTEGER := input_clk/reads_per_second;

-- COMPONENT test IS
-- 	PORT
-- 	(
-- 		clken		: IN STD_LOGIC ;
-- 		clock		: IN STD_LOGIC ;
-- 		denom		: IN STD_LOGIC_VECTOR (31 DOWNTO 0);
-- 		numer		: IN STD_LOGIC_VECTOR (31 DOWNTO 0);
-- 		quotient		: OUT STD_LOGIC_VECTOR (31 DOWNTO 0);
-- 		remain		: OUT STD_LOGIC_VECTOR (31 DOWNTO 0)
-- 	);
-- END COMPONENT test;

-- BME related constants
constant BME320_I2C_ADDR : std_logic_vector(6 downto 0) := "111011" & sdo;
constant START_ADDR       : std_logic_vector(7 downto 0) := x"F7"; -- memory address to start reading from on BME320
constant BME320_num_calibration : integer := 33;


signal read_i2c  : i2c_rec := (ena => '1', addr => BME320_I2C_ADDR, rw => '1', data_wr => (others => '0'));
signal write_i2c : i2c_rec := (ena => '1', addr => BME320_I2C_ADDR, rw => '0', data_wr => START_ADDR);
signal read_comp_i2c : i2c_rec;
signal initialize_BME280_i2c : i2c_rec := (ena => '1', addr => BME320_I2C_ADDR, rw => '0', data_wr => (others => '0'));
signal i2c_control : i2c_rec;

-- TODO: Set err, etc
-- Create states for the output state machine
type reader_state is (  idle, write, read, process_data, waiting, init, read_comp_data);
signal last_reader_state : reader_state := waiting;
signal cur_reader_state : reader_state := idle;
--signal nex_reader_state : reader_state := write;
signal write_complete   : std_logic := '0';
signal read_complete   : std_logic := '0';
signal process_data_complete   : std_logic := '0';
signal wait_complete   : boolean := false;
signal init_complete   : boolean := false;
signal compensation_data_received : boolean := false;

signal read_data       : std_logic_vector(63 downto 0) := (others => '0');


-- Data byte width definitions
constant temp_bit_width          : integer := 20;
constant humid_bit_width         : integer := 16;
constant pressure_bit_width      : integer := 20;
constant calibration_byte_width   : integer := 2;

-- BME word division definitions
constant temp_byte_location       : integer := 5;
constant humid_byte_location      : integer := 2;
constant pressure_byte_location   : integer := 8;

-- Signals for i2c control
-- signal i2c_enable : std_logic;
-- signal i2c_rdwr   : std_logic;
-- signal i2c_data_write : std_logic_vector(7 downto 0);
-- signal i2c_busy : std_logic;
-- signal i2c_data_read : std_logic_vector(7 downto 0);
-- signal i2c_err : std_logic;

-- Conversion variable used for pressure and humidity, assigned from temperature conversion
signal calibration_err : boolean;

-- TODO: Update for length to use byte width definition
signal temp_actual : signed(31 downto 0);
signal t_fine : signed(31 downto 0);
signal var1_s : signed(31 downto 0);
signal var2_s : signed(31 downto 0);
signal temp_raw_std     : std_logic_vector(temp_bit_width - 1 downto 0);

signal pressure_actual : unsigned(31 downto 0);
signal humid_actual    : unsigned(31 downto 0);
signal bytes_written   : integer range 0 to 7 := 0;
-- Calibration values for temperature, pressure, and humidity from the BME280
signal compensation_data : actual_cal_reg;
signal i2c_last        : std_logic := '1';
signal i2c_byte_began : boolean := false;
signal i2c_byte_finished : boolean := false;
begin

  -- test_inst : test PORT MAP (
	-- 	clken	 => '1'',
	-- 	clock	 => sys_clk,
	-- 	denom	 => denom_sig,
	-- 	numer	 => numer_sig,
	-- 	quotient	 => quotient_sig,
	-- 	remain	 => remain_sig
	-- );

  

bme_output_error(0) <= i2c_ack_error;


state_management : process (sys_clk, reset_n)
  variable wait_1 : integer := 0;
  variable i2c_initialized : boolean := false;
begin
  if reset_n = '0' then 
    cur_reader_state   <= idle;
    last_reader_state  <= waiting;
    busy_out <= '0';
  elsif rising_edge(sys_clk) then  
    last_reader_state <= cur_reader_state;
    case cur_reader_state is 
      when idle =>
        if i2c_busy = '0' then
          i2c_initialized := true;
        end if;
        if enable = '1' and i2c_initialized then
          busy_out <= '1';
          if compensation_data_received then
            cur_reader_state   <= write;
          else
            cur_reader_state   <= init;
          end if;
        else
          busy_out <= '0';
        end if;
      when init =>
        if init_complete then
          cur_reader_state <= read_comp_data;
        end if;
      when read_comp_data =>
        if compensation_data_received then
          cur_reader_state <= write;
        end if;
      when write =>
        if write_complete = '1' then
          cur_reader_state   <= read;
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

i2c_control_proc : process (sys_clk, reset_n)
  constant i2c_disable : i2c_rec := (ena => '0', addr => (others => '1'), rw => '1', data_wr => (others => '0'));
begin
  if reset_n = '0' then 
    i2c_control <= i2c_disable;
  elsif rising_edge(sys_clk) then

    i2c_last <= i2c_busy;
    i2c_byte_began <= i2c_last = '0' and i2c_busy = '1';
    i2c_byte_finished <= i2c_last = '1' and i2c_busy = '0';
    case cur_reader_state is
        when init =>
          i2c_control <= initialize_BME280_i2c; 
        when read_comp_data =>
          i2c_control <= read_comp_i2c;
        when write =>
          i2c_control <= write_i2c;
        when read =>
          i2c_control <= read_i2c;
        when others => 
          i2c_control <= i2c_disable;
    end case;
  end if;
end process;


i2c_ena <= i2c_control.ena;
i2c_addr <= i2c_control.addr;
i2c_rw   <= i2c_control.rw;
i2c_data_wr <= i2c_control.data_wr;



write_memory_address : process (sys_clk, reset_n, cur_reader_state, last_reader_state)
  -- when the state just became write
  variable start_write : boolean;
begin 
  if reset_n = '0' then 
    write_complete <= '0';
  elsif rising_edge(sys_clk) then
    start_write:= (cur_reader_state = write) and (last_reader_state /= write);
    if cur_reader_state = write then
      if i2c_byte_began then
        write_complete <= '1';
      else
        write_complete <= '0';
      end if;
    end if;
  end if;
end process;

read_from_BME320 : process (sys_clk, reset_n, cur_reader_state, last_reader_state)
  -- when the state just became read
  variable start_read : boolean;
  variable bytes_read : integer range -1 to read_data'length := -1;
  variable read_data_index : natural range 0 to 63;
  variable all_bytes_read : boolean := false;
  variable last_byte_to_read : boolean := false;
begin
  if reset_n = '0' then 
	 read_complete <= '0';
	 bytes_read := -1;
  elsif rising_edge(sys_clk) then
    if cur_reader_state = read then
      read_i2c.ena <= '1';
      last_byte_to_read := 8 * bytes_read = (read_data'length - 8);
        if(last_byte_to_read) then
          read_i2c.ena <= '0';
        end if;
      if i2c_byte_finished then
        read_data_index := read_data'length-8 * bytes_read;
        read_data(read_data_index - 1 downto (read_data_index - i2c_data_rd'length)) <= i2c_data_rd;
        bytes_read := bytes_read + 1;
  
        -- checks if bytes_read equals the number of bytes in read_data
         
        all_bytes_read := 8 * bytes_read = read_data'length; 
        if(all_bytes_read) then
          read_complete <= '1';
          bytes_read := -1;
        end if;
      end if;
	  else
	    read_complete <= '0';
    end if;
  end if;
end process;

process_data_from_BME320 : process (sys_clk, reset_n)
  variable temp_raw     : std_logic_vector(temp_bit_width - 1 downto 0);
  variable pressure_raw : std_logic_vector(pressure_bit_width - 1 downto 0);
  variable humid_raw    : std_logic_vector(humid_bit_width - 1 downto 0);
  variable temp_comp_results : cal_temp_vals;
  variable state : integer range 0 to 15 := 0;
  variable humid_temp : signed(31 downto 0);
  variable humid_temp2 : signed(31 downto 0);
  variable humid_calc_state : integer;
begin
  if reset_n = '0' then 
	 temp_actual     <= (others => '0');
	 t_fine          <= (others => '0');
    pressure_actual <= (others => '0');
    humid_actual    <= (others => '0');
    bme_output_data <= (others => '0');  
    bme_output_data <= std_logic_vector(compensation_data(humid_comp_byte_location + 1)) & std_logic_vector(compensation_data(humid_comp_byte_location + 5)) & std_logic_vector(humid_actual); 
  elsif rising_edge(sys_clk) then
    if cur_reader_state = process_data then
      case state is
        when 0 to 4 =>
          temp_raw := read_data(temp_byte_location * 8 - 1         downto (temp_byte_location * 8 - temp_bit_width));
          pressure_raw := read_data(pressure_byte_location * 8 - 1 downto (pressure_byte_location * 8 - pressure_bit_width));
          humid_raw := read_data(humid_byte_location * 8 - 1       downto (humid_byte_location * 8 - humid_bit_width));
      
          temp_comp_results := TempRawToActual(temp_raw, compensation_data(temp_comp_byte_location), 
                                              compensation_data(temp_comp_byte_location + 1), compensation_data(temp_comp_byte_location + 2));
          temp_actual <= temp_comp_results.temp_actual;
          t_fine      <= temp_comp_results.t_fine;
          var1_s      <= temp_comp_results.var1;
          var2_s      <= temp_comp_results.var2;
          state := state + 1;
     
        when 5 =>
          pressure_actual <= PressureRawToActual(pressure_raw, compensation_data(pressure_comp_byte_location), compensation_data(pressure_comp_byte_location + 1),
                                                compensation_data(pressure_comp_byte_location + 2), compensation_data(pressure_comp_byte_location + 3),
                                                compensation_data(pressure_comp_byte_location + 4), compensation_data(pressure_comp_byte_location + 5), 
                                                compensation_data(pressure_comp_byte_location + 6), compensation_data(pressure_comp_byte_location + 7), 
                                                compensation_data(pressure_comp_byte_location + 8), t_fine);
          state := state + 1;
          humid_calc_state := 0;
        when 6 => 
          HumidRawToActual(humid_raw, compensation_data(humid_comp_byte_location), compensation_data(humid_comp_byte_location + 1),
                                          compensation_data(humid_comp_byte_location + 2), compensation_data(humid_comp_byte_location + 3), 
                                          compensation_data(humid_comp_byte_location + 4), compensation_data(humid_comp_byte_location + 5), 
                                          t_fine, humid_calc_state, humid_temp, humid_temp2, humid_calc_state, humid_temp, humid_temp2, humid_actual);
          if(humid_calc_state > 7) then
            state := state + 1;
          end if;
        when 7 =>
          temp_raw_std <= temp_raw;
          pressure_actual <= (others => 0);
          bme_output_data <= std_logic_vector(temp_actual) & std_logic_vector(humid_actual) & std_logic_vector(pressure_actual); 
          --bme_output_data <= std_logic_vector(t_fine) & std_logic_vector(pressure_actual) & std_logic_vector(var2_s); 
          process_data_complete <= '1';
          state := state + 1;
        when others =>
          -- Do nothing
      end case;
    else
      process_data_complete <= '0';	 
      state := 0;
    end if;
  end if;
end process;

timing : process (sys_clk, reset_n)
    variable counter : integer := 0;
    variable waiting_ended : boolean := false;
begin
  if reset_n = '0' then 
	 wait_complete <= false;
	 counter := 0;
  elsif rising_edge(sys_clk) then
    waiting_ended := cur_reader_state /= waiting and last_reader_state = waiting;
    if wait_complete = true then
      -- Do nothing
    elsif counter = divider then
      counter := 0;
      wait_complete <= true;
    elsif waiting_ended then
      wait_complete <= false;
    else 
      counter := counter + 1;
    end if;
  end if;

end process;

-- TODO: Add writing 0x05 to 0xF2, 0x10 to 0xF5, and 0xB7 to 0xF4 in that order
initialize_BME280 : process (sys_clk, reset_n)
  type init_state is (idle, write_ctrl_hum, write_config, write_ctrl_measure, complete);
  constant SAMPLE_RATE_X16 : std_logic_vector(2 downto 0) := "101";
  constant NORMAL_MODE     : std_logic_vector(1 downto 0) := "11";
  constant FILTER_16       : std_logic_vector(2 downto 0) := "100";
  constant CTRL_HUM_ADDR   : std_logic_vector(7 downto 0) := x"F2";
  constant CONFIG_ADDR     : std_logic_vector(7 downto 0) := x"F5";
  constant CTRL_MEAS_ADDDR : std_logic_vector(7 downto 0) := x"F4"; 

  variable byte_complete   : boolean := false;
  
begin
  if reset_n = '0' then
    initialize_BME280_i2c.ena <= '0';
    initialize_BME280_i2c.data_wr <= (others => '0');
    bytes_written <= 0;
    init_complete <= false;
  elsif rising_edge(sys_clk) then
    if cur_reader_state = init then
      if i2c_byte_began then
        bytes_written <= bytes_written + 1;
      end if;
      
      case bytes_written is 
        when 0 =>
          initialize_BME280_i2c.ena <= '1';
          initialize_BME280_i2c.data_wr <= CTRL_HUM_ADDR;
        when 1 =>
          -- Setting humidity sampling rate, other bits do nothing
          initialize_BME280_i2c.data_wr <= "00000" & SAMPLE_RATE_X16;
        when 2 =>
          initialize_BME280_i2c.data_wr <= CONFIG_ADDR;
        when 3 => 
          -- Setting filter coefficient, first bits leave default stand by time and last bits leave SPI disabled
          initialize_BME280_i2c.data_wr <= "000" & FILTER_16 & "00";
        when 4 =>
          initialize_BME280_i2c.data_wr <= CTRL_MEAS_ADDDR;
        when 5 =>
          -- setting temp samping rate, pressure sampling rate, then putting device in normal mode 
          initialize_BME280_i2c.data_wr <= SAMPLE_RATE_X16 & SAMPLE_RATE_X16 & NORMAL_MODE;
        when others =>
          init_complete <= true;
        end case;
    end if;
  end if;
end process;

read_compensation_data_from_BME280 : process (sys_clk, reset_n, i2c_busy, i2c_data_rd)
    type compensation_data_reader_state is (idle, init_write, init_read, sec_write, sec_read, complete);
    constant INIT_COMP_ADDR : std_logic_vector(7 downto 0) := x"88";
    constant SEC_COMP_ADDR  : std_logic_vector(7 downto 0) := x"E1";
    constant INIT_BYTES     : integer := 25;
    constant SEC_BYTES      : integer := 7;

    variable raw_comp_regs   : raw_cal_reg;
	  variable bytes_read      : integer range -1 to 32 := -1;
	  variable all_bytes_read  : boolean;
	  variable cur_comp_data_reader_state : compensation_data_reader_state := idle;
    variable init_write_i2c : i2c_rec := (ena => '1', addr => BME320_I2C_ADDR, rw => '0', data_wr => INIT_COMP_ADDR);
    variable sec_write_i2c  : i2c_rec := (ena => '1', addr => BME320_I2C_ADDR, rw => '0', data_wr => SEC_COMP_ADDR);
    -- variable init_read_i2c  : i2c_rec := (ena => '1', addr => BME320_I2C_ADDR, rw => '1', data_wr => (others => '0'));
    variable last_byte_to_read : boolean := false;
    variable skip_A0 : boolean := true;
begin
  if reset_n = '0' then
    cur_comp_data_reader_state := idle;
    read_comp_i2c <= init_write_i2c;
    bytes_read := -1;
  elsif rising_edge(sys_clk) then
    if(cur_reader_state = read_comp_data) then
      case cur_comp_data_reader_state is
        when init_write =>
          read_comp_i2c <= init_write_i2c;
          if i2c_byte_began then
            cur_comp_data_reader_state := init_read;
          end if;
        when init_read =>
          read_comp_i2c <= read_i2c;
          last_byte_to_read := bytes_read = (INIT_BYTES - 1);
          if(last_byte_to_read and not(skip_A0)) then
            read_comp_i2c.ena <= '0';
          end if;
          if i2c_byte_finished then
            raw_comp_regs(bytes_read) := i2c_data_rd;
            bytes_read := bytes_read + 1;
            if(last_byte_to_read and skip_A0) then
              bytes_read := bytes_read - 1;
              skip_A0 := false;
            end if;

            -- checks if bytes_read equals the number of bytes in read_data
            all_bytes_read := bytes_read = INIT_BYTES; 
            if(all_bytes_read) then
              cur_comp_data_reader_state := sec_write;
            end if;
          end if;
        when sec_write =>
          read_comp_i2c <= sec_write_i2c;
          if i2c_byte_began then
            bytes_read := bytes_read - 1;
            cur_comp_data_reader_state := sec_read;
          end if;
        when sec_read =>
          read_comp_i2c <= read_i2c;
          last_byte_to_read := bytes_read = (INIT_BYTES + SEC_BYTES - 1);
          if(last_byte_to_read) then
            read_comp_i2c.ena <= '0';
          end if;
          if i2c_byte_finished then
            raw_comp_regs(bytes_read) := i2c_data_rd;
            bytes_read := bytes_read + 1;
          
            -- checks if bytes_read equals the number of bytes in read_data
            all_bytes_read := bytes_read = (INIT_BYTES + SEC_BYTES); 
            if(all_bytes_read) then
              cur_comp_data_reader_state := complete;
              compensation_data_received <= true;
              bytes_read := 0;

              compensation_data <= decode_comp_registers(cal_records, raw_comp_regs);
            end if;
          end if;
        when idle =>
          cur_comp_data_reader_state := init_write;
        when others =>
      end case;
    end if;
  end if;
     
end process;

end architecture FE_CPLD_BME280_I2C_Reader_arch;