library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;



-- Contains constants to initialize
package FE_BME280 is
  
  -- BME280 I2C ADDR constants
  constant BME320_Base_I2C_ADDR : std_logic_vector(5 downto 0) := "111011";
  constant BME320_I2C_ADDR_SDO_0 : std_logic_vector(6 downto 0) := BME320_Base_I2C_ADDR & '0';
  constant BME320_I2C_ADDR_SDO_1 : std_logic_vector(6 downto 0) := BME320_Base_I2C_ADDR & '1';
  
  -- Address to read sensors values from
  constant READ_PRESSURE_ADDR      : std_logic_vector(7 downto 0) := x"F7"; 
  constant READ_TEMP_ADDR      : std_logic_vector(7 downto 0) := x"FA"; 
  constant READ_HUMID_ADDR      : std_logic_vector(7 downto 0) := x"FD"; 
  constant READ_SENSOR_ADDR      : std_logic_vector(7 downto 0) := READ_PRESSURE_ADDR; 

  -- Number of compensation values on BME280
  constant BME280_num_calibration : integer := 18;
  
  -- Data byte width definitions for:
    -- ADC values
  constant temp_raw_bit_width          : integer := 20;
  constant humid_raw_bit_width         : integer := 16;
  constant pressure_raw_bit_width      : integer := 20;

  constant temp_byte_width          : integer := 4;
  constant humid_byte_width         : integer := 4;
  constant pressure_byte_width      : integer := 4;
  
    -- Compensation values
  constant calibration_byte_width   : integer := 4;
  constant temp_comp_byte_width     : integer := 3;
  constant pressure_comp_byte_width : integer := 9;
  constant humid_comp_byte_width    : integer := 6;
  constant bme_register_byte_width : integer := 1;

  -- BME compensation word division definitions
  constant temp_byte_location       : integer := 5;
  constant humid_byte_location      : integer := 2;
  constant pressure_byte_location   : integer := 8;
    -- continued, but counting up for compensation values
  constant temp_comp_byte_location : integer := 0;
  constant humid_comp_byte_location : integer := 12;
  constant pressure_comp_byte_location : integer := 3;

  -- Index value: 
  -- 0 is the upper bound of where to grab bits for the least significant bits
  -- 1 is the lower bound of where to grab bits for the least significant bits
  -- 2 is the upper bound of where to grab bits for the most significant bits
  -- 3 is the lower bound of where to grab bits for the most significant bits
  -- Ex: [7, 0, 15, 8] Compensation value = (15-8) & (7-0)
  -- Ex: [11, 8, 7, 0] Compensation value = (7-0) & (11-8)
  type bit_offset is array (3 downto 0) of integer;
  -- Type for holding the data to decode raw registers representing a single compensation values
  type cal_rec is record
      two_bytes : boolean;  -- True if the compensation value needs 2 registers to decode the value
      offset    : integer;  -- Index value in the array containing raw registers to start reading from. If two_bytes, reads offset and offset + 1
      is_signed : boolean;  -- True if the value should be interpreted as signed
      bit_offsets : bit_offset; -- Bit offsets on where to grab the least significant bits and then most significant bits for the compensation value
  end record cal_rec;

  -- Type for holding the data to decode the raw registers representing all of the compensation values
  type cal_recs is array ((BME280_num_calibration - 1) downto 0) of cal_rec;

  -- Used to return both t_fine and temp_actual from TempRawToActual
  type cal_temp_vals is record
    temp_actual : signed(temp_byte_width * 8 - 1 downto 0);
    t_fine      : signed(temp_byte_width * 8 - 1 downto 0);
  end record cal_temp_vals;
  
  constant cal_reg_raw_length : integer := 32;
  constant cal_reg_length : integer := 18;
  
  -- Type for holding the 8 bit register values that need to be pieced together to get the compensation values
  type raw_cal_reg is array (cal_reg_raw_length - 1 downto 0) of std_logic_vector(bme_register_byte_width * 8 - 1 downto 0);
  -- Type for holding the compensation values
  type actual_cal_reg is array(cal_reg_length - 1 downto 0) of signed(calibration_byte_width * 8 - 1 downto 0);
  
  
  -- Convert temperature from raw value to actual value with resolution 0.01 DegC. Output value of "5123" equals 51.23 DegC.
  -- Math is entirely based off BME 280 compensation formulas
  -- temp_raw : raw temperature data from BME 280
  -- dig_t1 : calibration value dig_t1 from BME 280
  -- dig_t2 : calibration value dig_t2 from BME 280
  -- dig_t3 : calibration value dig_t3 from BME 280
  -- Returns array of signed values, first value containing temp value and second containing t_fine
  function TempRawToActual(
    temp_raw_std : std_logic_vector(temp_raw_bit_width - 1 downto 0);
    dig_t1       : signed(calibration_byte_width * 8 - 1 downto 0);
    dig_t2       : signed(calibration_byte_width * 8 - 1 downto 0);
    dig_t3       : signed(calibration_byte_width * 8 - 1 downto 0)
    ) 
  return cal_temp_vals;

  -- Convert pressure from raw value to actual value
  -- Returns pressure in Pa in Q24.8 format (24 integer bits and 8 fractional bits).
  -- Output value of “24674867” represents 24674867/256 = 96386.2 Pa = 963.862 hPa
  -- Math is entirely based off BME 280 compensation formulas
  -- pressure_raw : raw presure data from BME 280
  -- dig_px : calibration value dig_px from BME 280 where x = 1 to 9
  -- t_fine     : fine temperature calibration value found during temperature compensation
  function PressureRawToActual(
    pressure_raw_std : std_logic_vector(pressure_raw_bit_width downto 0);
    dig_p1           : signed(calibration_byte_width * 8 - 1 downto 0);
    dig_p2           : signed(calibration_byte_width * 8 - 1 downto 0);
    dig_p3           : signed(calibration_byte_width * 8 - 1 downto 0);
    dig_p4           : signed(calibration_byte_width * 8 - 1 downto 0);
    dig_p5           : signed(calibration_byte_width * 8 - 1 downto 0);
    dig_p6           : signed(calibration_byte_width * 8 - 1 downto 0);
    dig_p7           : signed(calibration_byte_width * 8 - 1 downto 0);
    dig_p8           : signed(calibration_byte_width * 8 - 1 downto 0);
    dig_p9           : signed(calibration_byte_width * 8 - 1 downto 0);
    t_fine           : signed(temp_byte_width * 8 - 1 downto 0)
    )
  return unsigned;
                               
  -- Convert humidity from raw value to actual value
  -- Returns humidity in %RH in Q22.10 format (22 integer bits and 10 fractional bits).
  -- Output value of “47445” represents 47445/1024 = 46.333 %RH
  -- Math is entirely based off BME 280 compensation formulas
  -- humid_raw_std : raw temperature data from BME 280
  -- dig_px_std    : calibration value dig_px from BME 280 where x = 1 to 6
  -- t_fine        : fine temperature calibration value found during temperature compensation
  -- state         : determines what calculation should be done next
  -- temp          : temporary value for intermediate values in calculation
  -- temp2         : 2nd temporary value when needed for intermediate values in calculations
  -- humid_actual  : humidity in %RH in Q22.10 format
  procedure HumidRawToActual( 
    variable humid_raw_std  : in std_logic_vector(humid_raw_bit_width -1 downto 0);
    signal dig_h1           : in signed(calibration_byte_width * 8 - 1 downto 0);
    signal dig_h2           : in signed(calibration_byte_width * 8 - 1 downto 0);
    signal dig_h3           : in signed(calibration_byte_width * 8 - 1 downto 0);
    signal dig_h4           : in signed(calibration_byte_width * 8 - 1 downto 0);
    signal dig_h5           : in signed(calibration_byte_width * 8 - 1 downto 0);
    signal dig_h6           : in signed(calibration_byte_width * 8 - 1 downto 0);
    signal t_fine           : in signed(temp_byte_width * 8 - 1 downto 0);
    variable state_in       : in integer;
    variable temp_in        : in signed(31 downto 0);
    variable temp2_in       : in signed(31 downto 0);
    variable state_out      : out integer;
    variable temp_out       : out signed(31 downto 0);
    variable temp2_out      : out signed(31 downto 0);
    signal humid_actual     : out unsigned(humid_byte_width * 8 - 1 downto 0)
  );

-- Convert raw calibration register values to actual compensation values
-- Returns array of the 18 compensation values converted to signed
-- 0-2 is temp, 3-11 is pressure, and 12-17 is humid compensation values
-- pressure_raw : raw temperature data from BME 280
-- dig_px_std : calibration value dig_px from BME 280 where x = 1 to 6
-- t_fine     : fine temperature calibration value found durin
function decode_comp_registers(cal_decode_vals : cal_recs; cal_raw_regs : raw_cal_reg)
                               return actual_cal_reg;
									 
	-- Information to decode raw compensation registers
	-- Based off BME 280 compensation table
    constant cal_records : cal_recs := (
      0 => (
        two_bytes   => true,
        offset      => 0,
        is_signed   => false,
        bit_offsets => (
          0 => 7,
          1 => 0,
          2 => 15,
          3 => 8
        ) 
      ),
      1 => (
        two_bytes   => true,
        offset      => 2,
        is_signed   => true,
        bit_offsets => (
          0 => 7,
          1 => 0,
          2 => 15,
          3 => 8
        ) 
      ),
      2 => (
        two_bytes   => true,
        offset      => 4,
        is_signed   => true,
        bit_offsets => (
          0 => 7,
          1 => 0,
          2 => 15,
          3 => 8
        ) 
      ),
      3 => (
        two_bytes   => true,
        offset      => 6,
        is_signed   => false,
        bit_offsets => (
          0 => 7,
          1 => 0,
          2 => 15,
          3 => 8
        ) 
      ),
      4 => (
        two_bytes   => true,
        offset      => 8,
        is_signed   => true,
        bit_offsets => (
          0 => 7,
          1 => 0,
          2 => 15,
          3 => 8
        ) 
      ),
      5 => (
        two_bytes   => true,
        offset      => 10,
        is_signed   => true,
        bit_offsets => (
          0 => 7,
          1 => 0,
          2 => 15,
          3 => 8
        ) 
      ),
      6 => (
        two_bytes   => true,
        offset      => 12,
        is_signed   => true,
        bit_offsets => (
          0 => 7,
          1 => 0,
          2 => 15,
          3 => 8
        ) 
      ),
      7 => (
        two_bytes   => true,
        offset      => 14,
        is_signed   => true,
        bit_offsets => (
          0 => 7,
          1 => 0,
          2 => 15,
          3 => 8
        ) 
      ),
      8 => (
        two_bytes   => true,
        offset      => 16,
        is_signed   => true,
        bit_offsets => (
          0 => 7,
          1 => 0,
          2 => 15,
          3 => 8
        ) 
      ),
      9 => (
        two_bytes   => true,
        offset      => 18,
        is_signed   => true,
        bit_offsets => (
          0 => 7,
          1 => 0,
          2 => 15,
          3 => 8
        ) 
      ),
      10 => (
        two_bytes   => true,
        offset      => 20,
        is_signed   => true,
        bit_offsets => (
          0 => 7,
          1 => 0,
          2 => 15,
          3 => 8
        ) 
      ),
      11 => (
        two_bytes   => true,
        offset      => 22,
        is_signed   => true,
        bit_offsets => (
          0 => 7,
          1 => 0,
          2 => 15,
          3 => 8
        ) 
      ),
      12 => (
        two_bytes   => false,
        offset      => 24,
        is_signed   => false,
        bit_offsets => (
          0 => 7,
          1 => 0,
          2 => -1,
          3 => -1
        ) 
      ),
      13 => (
        two_bytes   => true,
        offset      => 25,
        is_signed   => true,
        bit_offsets => (
          0 => 7,
          1 => 0,
          2 => 15,
          3 => 8
        ) 
      ),
      14 => (
        two_bytes   => false,
        offset      => 27,
        is_signed   => false,
        bit_offsets => (
          0 => 7,
          1 => 0,
          2 => -1,
          3 => -1
        ) 
      ),
      15 => (
        two_bytes   => true,
        offset      => 28,
        is_signed   => true,
        bit_offsets => (
          0 => 11,
          1 => 8,
          2 => 7,
          3 => 0
        ) 
      ),
      16 => (
        two_bytes   => true,
        offset      => 29,
        is_signed   => true,
        bit_offsets => (
          0 => 3,
          1 => 0,
          2 => 11,
          3 => 4
        ) 
      ),
      17 => (
        two_bytes   => false,
        offset      => 31,
        is_signed   => true,
        bit_offsets => (
          0 => 7,
          1 => 0,
          2 => -1,
          3 => -1
        ) 
      )
    );
end package FE_BME280;

package body FE_BME280 is

-- Convert temperature from raw value to actual value with resolution 0.01 DegC. Output value of "5123" equals 51.23 DegC.
-- Math is entirely based off BME 280 compensation formulas
-- temp_raw : raw temperature data from BME 280
-- dig_t1_std : calibration value dig_t1 from BME 280
-- dig_t2_std : calibration value dig_t2 from BME 280
-- dig_t3_std : calibration value dig_t3 from BME 280
-- Returns array of signed values, first value containing temp value and second containing t_fine
function TempRawToActual(
  temp_raw_std : std_logic_vector(temp_raw_bit_width - 1 downto 0);
  dig_t1       : signed(calibration_byte_width * 8 - 1 downto 0);
  dig_t2       : signed(calibration_byte_width * 8 - 1 downto 0);
  dig_t3       : signed(calibration_byte_width * 8 - 1 downto 0)
  ) 
return cal_temp_vals is
  variable results     : cal_temp_vals;
  variable temp_raw    : signed(31 downto 0);
  variable temp_actual : signed(temp_byte_width * 8 - 1 downto 0);
  variable temp_fine   : signed(31 downto 0);
  variable var1        : signed(31 downto 0);
  variable var2        : signed(31 downto 0);
begin
  temp_raw := signed(resize(unsigned(temp_raw_std), temp_raw'length));
  
  var1 := resize(shift_right((shift_right(temp_raw, 3) - shift_left(dig_t1, 1)) * dig_t2, 11), var1'length);
  var2 := resize(shift_right(
    shift_right((shift_right(temp_raw, 4) - dig_t1) * (shift_right(temp_raw, 4) - dig_t1), 12)
    * dig_t3, 14), var2'length); 
  temp_fine := var1 + var2;
  temp_actual := resize(shift_right(temp_fine * 5 + 128, 8), temp_actual'length);

  results := (temp_actual => temp_actual, t_fine => temp_fine);

  return results;
end function;

-- Convert pressure from raw value to actual value
-- Returns pressure in Pa in Q24.8 format (24 integer bits and 8 fractional bits).
-- Output value of “24674867” represents 24674867/256 = 96386.2 Pa = 963.862 hPa
-- Math is entirely based off BME 280 compensation formulas
-- pressure_raw : raw temperature data from BME 280
-- dig_px_std : calibration value dig_px from BME 280 where x = 1 to 9
-- t_fine     : fine temperature calibration value found during temperature compensation
function PressureRawToActual(
  pressure_raw_std : std_logic_vector(pressure_raw_bit_width - 1 downto 0);
  dig_p1           : signed(calibration_byte_width * 8 - 1 downto 0);
  dig_p2           : signed(calibration_byte_width * 8 - 1 downto 0);
  dig_p3           : signed(calibration_byte_width * 8 - 1 downto 0);
  dig_p4           : signed(calibration_byte_width * 8 - 1 downto 0);
  dig_p5           : signed(calibration_byte_width * 8 - 1 downto 0);
  dig_p6           : signed(calibration_byte_width * 8 - 1 downto 0);
  dig_p7           : signed(calibration_byte_width * 8 - 1 downto 0);
  dig_p8           : signed(calibration_byte_width * 8 - 1 downto 0);
  dig_p9           : signed(calibration_byte_width * 8 - 1 downto 0);
  t_fine           : signed(temp_byte_width * 8 - 1 downto 0)
  )
return unsigned is
    variable pressure_actual : unsigned (pressure_byte_width * 8 - 1downto 0);
	  variable pressure_raw    : signed (31 downto 0);
    variable var1 : signed(31 downto 0);
    variable var2 : signed(31 downto 0);
    variable p    : unsigned(31 downto 0);
begin
  pressure_raw := resize(signed(pressure_raw_std), pressure_raw'length);

  var1 := shift_right(t_fine, 1) - to_signed(64000, var1'length);
  var2 := resize(shift_right(shift_right(var1, 2) * shift_right(var1, 2), 11) * dig_p6, var2'length);
  var2 := resize(var2 + shift_right(var1 * dig_p5, 1), var2'length);
  var2 := shift_right(var2, 2) + shift_right(dig_p4, 16);
  var1 := resize(shift_right(shift_right(shift_right(shift_right(var1, 2) * shift_right(var1, 2), 13) * dig_p3, 3) + shift_right(var1 * dig_p2, 1), 18), var1'length);
  var1 := resize(shift_right((to_signed(32768, 32) + var1) * dig_p1, 15), var1'length);

  if (var1 /= 0) then
    p := resize(unsigned(1048576 - pressure_raw - shift_right(var2, 12)) * 3125, p'length);
    
    -- TODO: Refactor to handle timing issue caused by division
    if (p < x"80000000") then
      --p := shift_right(p, 1) / unsigned(var1);
    else
      --p := resize((p / unsigned(var1)) * 2, p'length);
    end if;
    
    var1 := resize(shift_right(dig_p9 * signed(shift_right(shift_right(p, 3) * shift_right(p, 3), 13)), 12), var1'length);
    var2 := resize(shift_right(dig_p8 * signed(shift_right(p, 2)), 13), var2'length);
    p := resize(unsigned(signed(p) + shift_right(var1 + var2 + dig_p7, 4)), p'length);
    pressure_actual := p;
  else 
	-- TODO: handle error reporting
    -- calibration_err <= true;
    pressure_actual := to_unsigned(1, pressure_actual'length);
  end if;
  
  return pressure_actual;
end function;


-- Convert humidity from raw value to actual value
-- Returns humidity in %RH in Q22.10 format (22 integer bits and 10 fractional bits).
-- Output value of “47445” represents 47445/1024 = 46.333 %RH
-- Math is entirely based off BME 280 compensation formulas
-- humid_raw_std : raw temperature data from BME 280
-- dig_px_std    : calibration value dig_px from BME 280 where x = 1 to 6
-- t_fine        : fine temperature calibration value found during temperature compensation
-- state         : determines what calculation should be done next
-- temp          : temporary value for intermediate values in calculation
-- temp2         : 2nd temporary value when needed for intermediate values in calculations
-- humid_actual  : humidity in %RH in Q22.10 format
procedure HumidRawToActual( 
    variable humid_raw_std  : in std_logic_vector(humid_raw_bit_width - 1 downto 0);
    signal dig_h1         : in signed(calibration_byte_width * 8 - 1 downto 0);
    signal dig_h2         : in signed(calibration_byte_width * 8 - 1 downto 0);
    signal dig_h3         : in signed(calibration_byte_width * 8 - 1 downto 0);
    signal dig_h4         : in signed(calibration_byte_width * 8 - 1 downto 0);
    signal dig_h5         : in signed(calibration_byte_width * 8 - 1 downto 0);
    signal dig_h6         : in signed(calibration_byte_width * 8 - 1 downto 0);
    signal t_fine         : in signed(temp_byte_width * 8 - 1 downto 0);
    variable state_in     : in integer;
    variable temp_in      : in signed(31 downto 0);
    variable temp2_in     : in signed(31 downto 0);
    variable state_out    : out integer;
    variable temp_out     : out signed(31 downto 0);
    variable temp2_out    : out signed(31 downto 0);
    signal humid_actual   : out unsigned(humid_byte_width * 8 - 1 downto 0)
  ) is
    variable humid_raw    : signed(31 downto 0);
    variable v_x1_u32r    : signed(31 downto 0);
  begin

    humid_raw := resize(signed(humid_raw_std), humid_raw'length);
    
    case state_in is
      when 0 =>
        v_x1_u32r := t_fine - to_signed(76800, t_fine'length);
        temp_out := v_x1_u32r;
      when 1 =>
        v_x1_u32r := temp_in;
        temp_out := resize((shift_right( v_x1_u32r * dig_h3, 11)
        + to_signed(32768, 32)), 32);
        temp2_out := resize(shift_right( v_x1_u32r * dig_h6, 10), 32);
        
      when 2 =>
        temp_out := resize(temp2_in * temp_in, 32);

      when 3 =>
        v_x1_u32r := t_fine - to_signed(76800, t_fine'length);
        temp_out := resize(shift_right(temp_in, 10) + to_signed(2097152, 32), 32);

        
      when 4 =>
        v_x1_u32r := t_fine - to_signed(76800, t_fine'length);
        temp_out := resize(shift_right(temp_in * dig_h2 + to_signed(8192, 32), 14), temp_out'length);
        temp2_out := resize(shift_right(shift_left(humid_raw, 14) - shift_left(dig_h4, 20) - dig_h5 * v_x1_u32r +
        to_signed(16384, 32), 15), temp2_out'length);
      when 5 =>
        v_x1_u32r := t_fine - to_signed(76800, t_fine'length);
        v_x1_u32r := resize( temp2_in * temp_in, v_x1_u32r'length);
        temp_out := v_x1_u32r;
      when 6 =>
        v_x1_u32r := temp_in;
        
        if v_x1_u32r < 0 then
          v_x1_u32r := to_signed(0, v_x1_u32r'length);
        elsif v_x1_u32r > 419430400 then
          v_x1_u32r :=  to_signed(419430400, v_x1_u32r'length);
        end if;
        temp_out := v_x1_u32r;
      when 7 =>
        v_x1_u32r := temp_in;
        temp_out := shift_right(v_x1_u32r, 12);
        humid_actual <= unsigned(shift_right(v_x1_u32r, 12));
      when others =>
        -- do nothing
    end case;
    state_out := state_in + 1;

  end procedure;

-- Convert raw calibration register values to actual compensation values
-- Returns array of the 18 compensation values converted to signed
-- 0-2 is temp, 3-11 is pressure, and 12-17 is humid compensation values
-- pressure_raw : raw temperature data from BME 280
-- dig_px_std : calibration value dig_px from BME 280 where x = 1 to 6
-- t_fine     : fine temperature calibration value found durin
function decode_comp_registers(cal_decode_vals : cal_recs; cal_raw_regs : raw_cal_reg)
										 return actual_cal_reg is
  variable actual_cal_regs   : actual_cal_reg;
  variable temp_cal_rec  : cal_rec;
  variable cal_sorted    : std_logic_vector( 15 downto 0);
  variable raw_cal       : std_logic_vector( 15 downto 0);
  variable offset_length : integer;
  variable offset_length_2 : integer;
  variable actual_cal    : signed(calibration_byte_width * 8 - 1 downto 0);
  variable is_12bit      : boolean;
begin
	 for cal_ind in cal_reg_length - 1 downto 0 loop
		temp_cal_rec := cal_decode_vals(cal_ind);
		
    if(temp_cal_rec.two_bytes) then 
      -- Setting the first register read as the LSB. Ex: Setting raw_cal = 0x89 & 0x88
			raw_cal := cal_raw_regs(temp_cal_rec.offset + 1) & cal_raw_regs( temp_cal_rec.offset);
      
      -- Calculate how many bits are being set
      offset_length := temp_cal_rec.bit_offsets(0) - temp_cal_rec.bit_offsets(1);
      offset_length_2 :=  temp_cal_rec.bit_offsets(2) - temp_cal_rec.bit_offsets(3)  + offset_length + 1;
			
      cal_sorted := (others => '0');
      -- Set least significant (4 or 8) bits of the value
			cal_sorted(offset_length downto 0) := raw_cal(temp_cal_rec.bit_offsets(0) downto temp_cal_rec.bit_offsets(1));
      -- Set most significant bits of the value
      cal_sorted(offset_length_2 downto offset_length + 1) := raw_cal(temp_cal_rec.bit_offsets(2) downto temp_cal_rec.bit_offsets(3));
			
			if temp_cal_rec.is_signed then
				is_12bit := temp_cal_rec.bit_offsets(0) = 11 or temp_cal_rec.bit_offsets(2) = 11;
				if is_12bit then
				  actual_cal := resize(signed(cal_sorted(11 downto 0)), calibration_byte_width * 8);
				else
				  actual_cal := resize(signed(cal_sorted), calibration_byte_width * 8);
				end if;
      else
        -- If unsigned, interpret the initial value as unsigned before converting to signed
			  actual_cal := signed(resize(unsigned(cal_sorted), calibration_byte_width * 8));
			end if;
		else -- cal is a single byte
		  if temp_cal_rec.is_signed then
		    actual_cal := resize(signed(cal_raw_regs(temp_cal_rec.offset)), calibration_byte_width * 8);
      else
        -- If unsigned, interpret the initial value as unsigned before converting to signed
		    actual_cal := signed(resize(unsigned(cal_raw_regs(temp_cal_rec.offset)), calibration_byte_width * 8));
		  end if;
		end if;
		actual_cal_regs(cal_ind) := actual_cal;
	end loop;
	
	return actual_cal_regs;
end function;

end package body FE_BME280;