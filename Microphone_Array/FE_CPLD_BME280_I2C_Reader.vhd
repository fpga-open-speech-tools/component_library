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
signal temp_fine : signed(31 downto 0);
signal calibration_err : boolean;

-- TODO: Update for length to use byte width definition
signal temp_actual : signed(31 downto 0);
signal pressure_actual : unsigned(31 downto 0);
signal humid_actual    : unsigned(31 downto 0);

-- Calibration values for temperature, from the BME280
dig_t1_std : std_logic_vector(calibration_byte_width * 8 - 1 downto 0);
dig_t2_std : std_logic_vector(calibration_byte_width * 8 - 1 downto 0);
dig_t3_std : std_logic_vector(calibration_byte_width * 8 - 1 downto 0);

-- Calibration values for pressure, from the BME280
dig_p1_std : std_logic_vector(calibration_byte_width * 8 - 1 downto 0);
dig_p2_std : std_logic_vector(calibration_byte_width * 8 - 1 downto 0);
dig_p3_std : std_logic_vector(calibration_byte_width * 8 - 1 downto 0);
dig_p4_std : std_logic_vector(calibration_byte_width * 8 - 1 downto 0);
dig_p5_std : std_logic_vector(calibration_byte_width * 8 - 1 downto 0);
dig_p6_std : std_logic_vector(calibration_byte_width * 8 - 1 downto 0);
dig_p7_std : std_logic_vector(calibration_byte_width * 8 - 1 downto 0);
dig_p8_std : std_logic_vector(calibration_byte_width * 8 - 1 downto 0);
dig_p9_std : std_logic_vector(calibration_byte_width * 8 - 1 downto 0);

-- Calibrations values for humidity, from the BME280
dig_h1_std : std_logic_vector(calibration_byte_width * 8 - 1 downto 0);
dig_h2_std : std_logic_vector(calibration_byte_width * 8 - 1 downto 0);
dig_h3_std : std_logic_vector(calibration_byte_width * 8 - 1 downto 0);
dig_h4_std : std_logic_vector(calibration_byte_width * 8 - 1 downto 0);
dig_h5_std : std_logic_vector(calibration_byte_width * 8 - 1 downto 0);
dig_h6_std : std_logic_vector(calibration_byte_width * 8 - 1 downto 0);

type bit_offset is array (3 downto 0) of integer;

type cal_rec is record
  two_bytes : boolean;
  offset    : integer;
  is_signed : boolean;
  bit_offsets : bit_offset;
end record cal_rec;

type cal_recs is array ((BME320_num_calibration - 1) downto 0) of cal_rec;
constant cal_recs : cal_recs := (
  0 => (
    two_bytes   <= true;
    offset      <= 0;
    is_signed   <= false;
    bit_offsets <= (
      0 => 7,
      1 => 0,
      2 => 15,
      3 => 8,
    ) ;
  ),
  1 => (
    two_bytes   <= true;
    offset      <= 2;
    is_signed   <= true;
    bit_offsets <= (
      0 => 7,
      1 => 0,
      2 => 15,
      3 => 8,
    ) ;
  ),
  2 => (
    two_bytes   <= true;
    offset      <= 4;
    is_signed   <= true;
    bit_offsets <= (
      0 => 7,
      1 => 0,
      2 => 15,
      3 => 8,
    ) ;
  ),
  3 => (
    two_bytes   <= true;
    offset      <= 6;
    is_signed   <= false;
    bit_offsets <= (
      0 => 7,
      1 => 0,
      2 => 15,
      3 => 8,
    ) ;
  ),
  4 => (
    two_bytes   <= true;
    offset      <= 8;
    is_signed   <= true;
    bit_offsets <= (
      0 => 7,
      1 => 0,
      2 => 15,
      3 => 8,
    ) ;
  ),
  5 => (
    two_bytes   <= true;
    offset      <= 10;
    is_signed   <= true;
    bit_offsets <= (
      0 => 7,
      1 => 0,
      2 => 15,
      3 => 8,
    ) ;
  ),
  6 => (
    two_bytes   <= true;
    offset      <= 12;
    is_signed   <= true;
    bit_offsets <= (
      0 => 7,
      1 => 0,
      2 => 15,
      3 => 8,
    ) ;
  ),
  7 => (
    two_bytes   <= true;
    offset      <= 14;
    is_signed   <= true;
    bit_offsets <= (
      0 => 7,
      1 => 0,
      2 => 15,
      3 => 8,
    ) ;
  ),
  8 => (
    two_bytes   <= true;
    offset      <= 16;
    is_signed   <= true;
    bit_offsets <= (
      0 => 7,
      1 => 0,
      2 => 15,
      3 => 8,
    ) ;
  ),
  9 => (
    two_bytes   <= true;
    offset      <= 18;
    is_signed   <= true;
    bit_offsets <= (
      0 => 7,
      1 => 0,
      2 => 15,
      3 => 8,
    ) ;
  ),
  10 => (
    two_bytes   <= true;
    offset      <= 20;
    is_signed   <= true;
    bit_offsets <= (
      0 => 7,
      1 => 0,
      2 => 15,
      3 => 8,
    ) ;
  ),
  11 => (
    two_bytes   <= true;
    offset      <= 22;
    is_signed   <= true;
    bit_offsets <= (
      0 => 7,
      1 => 0,
      2 => 15,
      3 => 8,
    ) ;
  ),
  12 => (
    two_bytes   <= false;
    offset      <= 24;
    is_signed   <= false;
    bit_offsets <= (
      0 => 7,
      1 => 0,
      2 => -1,
      3 => -1,
    ) ;
  ),
  13 => (
    two_bytes   <= true;
    offset      <= 25;
    is_signed   <= true;
    bit_offsets <= (
      0 => 7,
      1 => 0,
      2 => 15,
      3 => 8,
    ) ;
  ),
  14 => (
    two_bytes   <= false;
    offset      <= 27;
    is_signed   <= false;
    bit_offsets <= (
      0 => 7,
      1 => 0,
      2 => -1,
      3 => -1,
    ) ;
  ),
  15 => (
    two_bytes   <= true;
    offset      <= 28;
    is_signed   <= true;
    bit_offsets <= (
      0 => 11,
      1 => 4,
      2 => 3,
      3 => 0,
    ) ;
  ),
  16 => (
    two_bytes   <= true;
    offset      <= 30;
    is_signed   <= true;
    bit_offsets <= (
      0 => 3,
      1 => 0,
      2 => 11,
      3 => 4,
    ) ;
  ),
  17 => (
    two_bytes   <= false;
    offset      <= 32;
    is_signed   <= true;
    bit_offsets <= (
      0 => 7,
      1 => 0,
      2 => -1,
      3 => -1,
    ) ;
  ),
);


-- Convert temperature from raw value to actual value with resolution 0.01 DegC. Output value of "5123" equals 51.23 DegC.
-- Math is entirely based off BME 280 compensation formulas
-- temp_raw : raw temperature data from BME 280
-- dig_t1_std : calibration value dig_t1 from BME 280
-- dig_t2_std : calibration value dig_t2 from BME 280
-- dig_t3_std : calibration value dig_t3 from BME 280
impure function TempRawToActual(temp_raw : std_logic_vector(temp_byte_width * 8 -1 downto 0);
                                dig_t1_std : std_logic_vector(calibration_byte_width * 8 - 1 downto 0);
                                dig_t2_std : std_logic_vector(calibration_byte_width * 8 - 1 downto 0);
                                dig_t3_std : std_logic_vector(calibration_byte_width * 8 - 1 downto 0)) 
                                return signed (31 downto 0) is
    variable temp_actual : signed(31 downto 0);
    variable var1        : signed(31 downto 0);
    variable var2        : signed(31 downto 0);
    variable dig_t1      : signed(31 downto 0);
    variable dig_t2      : signed(31 downto 0);
    variable dig_t3      : signed(31 downto 0);
begin
  dig_t1 := to_signed(dig_t1_std, dig_t1'length); 
  dig_t2 := to_signed(dig_t2_std, dig_t2'length);
  dig_t3 := to_signed(dig_t3_std, dig_t3'length);  
  
  var1 := (shift_right(temp_raw, 3) - shift_left(dig_t1, 3)) * shift_right(dig_t2, 11);
  var2 := shift_right(
            shift_right(shift_right(temp_raw, 4) - dig_t1) * (shift_right(temp_raw, 4) - dig_t1), 12)
            * dig_t3, 14); 
  temp_fine <= var1 + var2;
  temp_actual := shift_right(t_fine * 5 + 128, 8);

  return temp_actual;
end function;

-- Convert pressure from raw value to actual value
-- Returns pressure in Pa in Q24.8 format (24 integer bits and 8 fractional bits).
-- Output value of “24674867” represents 24674867/256 = 96386.2 Pa = 963.862 hPa
-- Math is entirely based off BME 280 compensation formulas
-- pressure_raw : raw temperature data from BME 280
-- dig_px_std : calibration value dig_px from BME 280 where x = 1 to 9
-- t_fine     : fine temperature calibration value found during temperature compensation
function PressureRawToActual(pressure_raw : std_logic_vector(pressure_byte_width * 8 - 1 downto 0);
                             dig_p1_std : std_logic_vector(calibration_byte_width * 8 - 1 downto 0);
                             dig_p2_std : std_logic_vector(calibration_byte_width * 8 - 1 downto 0);
                             dig_p3_std : std_logic_vector(calibration_byte_width * 8 - 1 downto 0);
                             dig_p4_std : std_logic_vector(calibration_byte_width * 8 - 1 downto 0);
                             dig_p5_std : std_logic_vector(calibration_byte_width * 8 - 1 downto 0);
                             dig_p6_std : std_logic_vector(calibration_byte_width * 8 - 1 downto 0);
                             dig_p7_std : std_logic_vector(calibration_byte_width * 8 - 1 downto 0);
                             dig_p8_std : std_logic_vector(calibration_byte_width * 8 - 1 downto 0);
                             dig_p9_std : std_logic_vector(calibration_byte_width * 8 - 1 downto 0);
                             t_fine     : signed(31 downto 0);
                             )
                             return unsigned (31 downto 0) is
    variable pressure_actual : unsigned (31 downto 0);
    variable var1 : signed(63 downto 0);
    variable var2 : signed(63 downto 0);
    variable p    : signed(63 downto 0);
    variable temp_fine_64 : signed(63 downto 0);
    variable dig_p1      : signed(31 downto 0);
    variable dig_p2      : signed(31 downto 0);
    variable dig_p3      : signed(31 downto 0);
    variable dig_p4      : signed(31 downto 0);
    variable dig_p5      : signed(31 downto 0);
    variable dig_p6      : signed(31 downto 0);
    variable dig_p7      : signed(31 downto 0);
    variable dig_p8      : signed(31 downto 0);
    variable dig_p9      : signed(31 downto 0);
    -- TODO: fix pressure raw being std_logic but expecting to be S32
begin
  dig_p1 := to_signed(dig_p1_std, dig_p1'length); 
  dig_p2 := to_signed(dig_p2_std, dig_p2'length);
  dig_p3 := to_signed(dig_p3_std, dig_p3'length);
  dig_p4 := to_signed(dig_p4_std, dig_p4'length); 
  dig_p5 := to_signed(dig_p5_std, dig_p5'length);
  dig_p6 := to_signed(dig_p6_std, dig_p6'length); 
  dig_p7 := to_signed(dig_p7_std, dig_p7'length); 
  dig_p8 := to_signed(dig_p8_std, dig_p8'length);
  dig_p9 := to_signed(dig_p9_std, dig_p9'length);  
  temp_fine_64 := resize(t_fine, temp_fine_64'length);

  var1 := temp_fine_64 - to_signed(128000, var1'length);
  var2 := resize(var1 * var1 * dig_p6, var2'length);
  var2 := var2 + shift_left(var1 * dig_p5, 17);
  var2 := var2 + shift_left(dig_p4, 35);
  var1 := shift_right(var1 * var1 * dig_p3, 8) + shift_left(var1 * dig_p2);
  var1 := shift_right((shift_left(to_signed(1, 64), 47 + var1) * dig_p1, 33);

  if (var1 != 0) then
    p := 1048576 - pressure_raw;
    p := ((shift_left(p, 31) - var2) * 3125) / var1;
    var1 := shift_right(dig_p9 * shift_right(p, 13) * shift_right(p, 13), 25);
    var2 := shift_right(dig_p8 * p, 19);
    p := shift_right(p + var1 + var2, 8) + shift_left(dig_p7, 4);
    pressure_actual := to_unsigned(p, pressure_actual'length);
  else 
    calibration_err <= true;
    pressure_actual := to_unsigned(0, pressure_actual'length);
  end if;

  return pressure_actual;
end function;

-- Convert pressure from raw value to actual value
-- Returns pressure in %RH in Q22.10 format (22 integer bits and 10 fractional bits).
-- Output value of “47445” represents 47445/1024 = 46.333 %RH
-- Math is entirely based off BME 280 compensation formulas
-- pressure_raw : raw temperature data from BME 280
-- dig_px_std : calibration value dig_px from BME 280 where x = 1 to 6
-- t_fine     : fine temperature calibration value found during temperature compensation
function HumidRawToActual(humid_raw : std_logic_vector(humid_byte_width * 8 -1 downto 0);
                          dig_h1_std : std_logic_vector(calibration_byte_width * 8 - 1 downto 0);
                          dig_h2_std : std_logic_vector(calibration_byte_width * 8 - 1 downto 0);
                          dig_h3_std : std_logic_vector(calibration_byte_width * 8 - 1 downto 0);
                          dig_h4_std : std_logic_vector(calibration_byte_width * 8 - 1 downto 0);
                          dig_h5_std : std_logic_vector(calibration_byte_width * 8 - 1 downto 0);
                          dig_h6_std : std_logic_vector(calibration_byte_width * 8 - 1 downto 0);
                          t_fine     : signed(31 downto 0);
                          )
                          return unsigned (31 downto 0) is
    variable humid_actual : unsigned (31 downto 0);
    variable dig_h1       : signed(31 downto 0);
    variable dig_h2       : signed(31 downto 0);
    variable dig_h3       : signed(31 downto 0);
    variable dig_h4       : signed(31 downto 0);
    variable dig_h5       : signed(31 downto 0);
    variable dig_h6       : signed(31 downto 0);
    variable v_x1_u32r    : signed(31 downto 0);
begin
  dig_h1 := to_signed(dig_h1_std, dig_h1'length); 
  dig_h2 := to_signed(dig_h2_std, dig_h2'length);
  dig_h3 := to_signed(dig_h3_std, dig_h3'length);
  dig_h4 := to_signed(dig_h4_std, dig_h4'length); 
  dig_h5 := to_signed(dig_h5_std, dig_h5'length);
  dig_h6 := to_signed(dig_h6_std, dig_h6'length); 

  v_x1_u32r := t_fine - to_signed(76800, t_fine'length);
  v_x1_u32r := shift_left(humid_raw, 14) - (shift_left(dig_h4, 20) - dig_h5 * v_c1_u32r) +
               shift_right(to_signed(16384, 32), 15) * (
                shift_right( 
                  (
                    shift_right(
                      shift_right( v_x1_u32r * dig_h6, 10) * shift_right( v_x1_u32r * dig_h3)
                      + to_signed(32768, 32), 10)
                    + to_signed(2097152, 32))
                  * dig_h2 + to_signed(8192, 32), 14);
               );
  v_x1_u32r := (v_x1_u32r - shift_right(
                                        shift_right(
                                                    shift_right(v_x1_u32r, 15) * shift_right(v_x1_u32r, 15)
                                                    , 7) * dig_h1
                                        , 4)
               );
  if v_x1_u32r < 0 then
    v_x1_u32r := 0;
  elsif v_x1_u32r > 419430400 then
    v_x1_u32r :=  419430400;
  end if;
  
  humid_actual := to_unsigned(shift_right(v_x1_u32r, 12));
  return humid_actual;
end function;



begin

i2c_component : i2c_master 
generic map (
  input_clk => FE_CPLD_BME280_I2C_Reader.input_clk,  
  bus_clk   => FE_CPLD_BME280_I2C_Reader.bus_clk,
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
  scl       => FE_CPLD_BME280_I2C_Reader.scl,
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
            cur_reader_state   <= read_compensation_data
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
  constant i2c_disable : i2c_rec := (ena => '0', addr => (others => '1'), rw => '1', data_wr => (others => '0'))
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
  variable start_write : boolean := cur_reader_state = write && last_reader_state != write;
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
  variable start_read : boolean := cur_reader_state = read && last_reader_state != read;
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
  waiting_ended := cur_reader_state != waiting && last_reader_state = waiting;
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

            assign_data : for I in 0 to N-1 generate
            D_Flip_Flop :
            D_FF port map
                 (S(I+1), S(I), Q(I), S(I+1));
            end generate;

          end if;
        end if;
      when idle =>
        -- Do nothing
      when others =>
    end case;
  end if;
     
end process;

end architecture FE_CPLD_BME280_I2C_Reader_arch;