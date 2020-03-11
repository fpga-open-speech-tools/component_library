library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

package FE_CPLD_BME280_Compensate_Data is

  constant BME320_num_calibration : integer := 18;
  
  -- Data byte width definitions
  constant temp_byte_width          : integer := 3;
  constant humid_byte_width         : integer := 2;
  constant pressure_byte_width      : integer := 3;
  constant calibration_byte_width   : integer := 4;
  constant temp_comp_byte_width     : integer := 3;
  constant pressure_comp_byte_width : integer := 9;
  constant humid_comp_byte_width    : integer := 6;
  
  -- BME compensation word division definitions, counting up
  constant temp_comp_byte_location : integer := 0;
  constant humid_comp_byte_location : integer := 12;
  constant pressure_comp_byte_location : integer := 3;

  type bit_offset is array (3 downto 0) of integer;
  type cal_rec is record
      two_bytes : boolean;
      offset    : integer;
      is_signed : boolean;
      bit_offsets : bit_offset;
  end record cal_rec;

  type cal_recs is array ((BME320_num_calibration - 1) downto 0) of cal_rec;

  -- Used to return both t_fine and temp_actual from TempRawToActual
  type cal_temp_vals is record
    temp_actual : signed(31 downto 0);
    t_fine      : signed(31 downto 0);
    var1        : signed(31 downto 0);
    var2        : signed(31 downto 0);
  end record cal_temp_vals;
  
  constant cal_reg_raw_length : integer := 32;
  constant cal_reg_raw_byte_width : integer := 1;
  constant cal_reg_length : integer := 18;
  constant cal_reg_byte_width : integer := 4;
  
  type raw_cal_reg is array (cal_reg_raw_length - 1 downto 0) of std_logic_vector(cal_reg_raw_byte_width * 8 - 1 downto 0);
  -- TODO: Refactor this to be signed instead of std_logic_vector
  type actual_cal_reg is array(cal_reg_length - 1 downto 0) of signed(cal_reg_byte_width * 8 - 1 downto 0);
  
  
  -- Convert temperature from raw value to actual value with resolution 0.01 DegC. Output value of "5123" equals 51.23 DegC.
  -- Math is entirely based off BME 280 compensation formulas
  -- temp_raw : raw temperature data from BME 280
  -- dig_t1 : calibration value dig_t1 from BME 280
  -- dig_t2 : calibration value dig_t2 from BME 280
  -- dig_t3 : calibration value dig_t3 from BME 280
  -- Returns array of signed values, first value containing temp value and second containing t_fine
  function TempRawToActual(temp_raw_std : std_logic_vector(19 downto 0);
                                  dig_t1 : signed(calibration_byte_width * 8 - 1 downto 0);
                                  dig_t2 : signed(calibration_byte_width * 8 - 1 downto 0);
                                  dig_t3 : signed(calibration_byte_width * 8 - 1 downto 0)) 
                                  return cal_temp_vals;

  -- Convert pressure from raw value to actual value
  -- Returns pressure in Pa in Q24.8 format (24 integer bits and 8 fractional bits).
  -- Output value of “24674867” represents 24674867/256 = 96386.2 Pa = 963.862 hPa
  -- Math is entirely based off BME 280 compensation formulas
  -- pressure_raw : raw presure data from BME 280
  -- dig_px : calibration value dig_px from BME 280 where x = 1 to 9
  -- t_fine     : fine temperature calibration value found during temperature compensation
  function PressureRawToActual(pressure_raw_std : std_logic_vector(19 downto 0);
                               dig_p1 : signed(calibration_byte_width * 8 - 1 downto 0);
                               dig_p2 : signed(calibration_byte_width * 8 - 1 downto 0);
                               dig_p3 : signed(calibration_byte_width * 8 - 1 downto 0);
                               dig_p4 : signed(calibration_byte_width * 8 - 1 downto 0);
                               dig_p5 : signed(calibration_byte_width * 8 - 1 downto 0);
                               dig_p6 : signed(calibration_byte_width * 8 - 1 downto 0);
                               dig_p7 : signed(calibration_byte_width * 8 - 1 downto 0);
                               dig_p8 : signed(calibration_byte_width * 8 - 1 downto 0);
                               dig_p9 : signed(calibration_byte_width * 8 - 1 downto 0);
                               t_fine     : signed(31 downto 0)
                               )
                               return unsigned;
                               
  -- Convert pressure from raw value to actual value
  -- Returns pressure in %RH in Q22.10 format (22 integer bits and 10 fractional bits).
  -- Output value of “47445” represents 47445/1024 = 46.333 %RH
  -- Math is entirely based off BME 280 compensation formulas
  -- humid_raw : raw humidity data from BME 280
  -- dig_hx : calibration value dig_hx from BME 280 where x = 1 to 6
  -- t_fine     : fine temperature calibration value found during temperature compensation
  function HumidRawToActual(humid_raw_std : std_logic_vector(humid_byte_width * 8 -1 downto 0);
                            dig_h1 : signed(calibration_byte_width * 8 - 1 downto 0);
                            dig_h2 : signed(calibration_byte_width * 8 - 1 downto 0);
                            dig_h3 : signed(calibration_byte_width * 8 - 1 downto 0);
                            dig_h4 : signed(calibration_byte_width * 8 - 1 downto 0);
                            dig_h5 : signed(calibration_byte_width * 8 - 1 downto 0);
                            dig_h6 : signed(calibration_byte_width * 8 - 1 downto 0);
                            t_fine     : signed(31 downto 0)
                            )
                            return unsigned;

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
end package FE_CPLD_BME280_Compensate_Data;

package body FE_CPLD_BME280_Compensate_Data is

-- Convert temperature from raw value to actual value with resolution 0.01 DegC. Output value of "5123" equals 51.23 DegC.
-- Math is entirely based off BME 280 compensation formulas
-- temp_raw : raw temperature data from BME 280
-- dig_t1_std : calibration value dig_t1 from BME 280
-- dig_t2_std : calibration value dig_t2 from BME 280
-- dig_t3_std : calibration value dig_t3 from BME 280
-- Returns array of signed values, first value containing temp value and second containing t_fine
function TempRawToActual(temp_raw_std : std_logic_vector(19 downto 0);
                                dig_t1 : signed(calibration_byte_width * 8 - 1 downto 0);
                                dig_t2 : signed(calibration_byte_width * 8 - 1 downto 0);
                                dig_t3 : signed(calibration_byte_width * 8 - 1 downto 0)) 
                                return cal_temp_vals is
  variable results     : cal_temp_vals;
  variable temp_raw    : signed(31 downto 0);
  variable temp_actual : signed(31 downto 0);
  variable temp_fine   : signed(31 downto 0);
  variable var1        : signed(31 downto 0);
  variable var2        : signed(31 downto 0);
begin
  temp_raw := signed(resize(unsigned(temp_raw_std), temp_raw'length));
  
  var1 := resize(shift_right((shift_right(temp_raw, 3) - shift_left(dig_t1, 1)) * dig_t2, 11), var1'length);
  --var1 := shift_right(temp_raw, 3);
  --var2 := shift_left(dig_t1, 1);
  var2 := resize(shift_right(
    shift_right((shift_right(temp_raw, 4) - dig_t1) * (shift_right(temp_raw, 4) - dig_t1), 12)
    * dig_t3, 14), var2'length); 
  temp_fine := var1 + var2;
  temp_actual := resize(shift_right(temp_fine * 5 + 128, 8), temp_actual'length);

  results := (temp_actual => temp_actual, t_fine => temp_fine, var1 => var1, var2 => var2);

  return results;
end function;

-- Convert pressure from raw value to actual value
-- Returns pressure in Pa in Q24.8 format (24 integer bits and 8 fractional bits).
-- Output value of “24674867” represents 24674867/256 = 96386.2 Pa = 963.862 hPa
-- Math is entirely based off BME 280 compensation formulas
-- pressure_raw : raw temperature data from BME 280
-- dig_px_std : calibration value dig_px from BME 280 where x = 1 to 9
-- t_fine     : fine temperature calibration value found during temperature compensation
function PressureRawToActual(pressure_raw_std : std_logic_vector(19 downto 0);
                             dig_p1 : signed(calibration_byte_width * 8 - 1 downto 0);
                             dig_p2 : signed(calibration_byte_width * 8 - 1 downto 0);
                             dig_p3 : signed(calibration_byte_width * 8 - 1 downto 0);
                             dig_p4 : signed(calibration_byte_width * 8 - 1 downto 0);
                             dig_p5 : signed(calibration_byte_width * 8 - 1 downto 0);
                             dig_p6 : signed(calibration_byte_width * 8 - 1 downto 0);
                             dig_p7 : signed(calibration_byte_width * 8 - 1 downto 0);
                             dig_p8 : signed(calibration_byte_width * 8 - 1 downto 0);
                             dig_p9 : signed(calibration_byte_width * 8 - 1 downto 0);
                             t_fine     : signed(31 downto 0)
                             )
                             return unsigned is
    variable pressure_actual : unsigned (31 downto 0);
	  variable pressure_raw    : signed (31 downto 0);
    variable var1 : signed(63 downto 0);
    variable var2 : signed(63 downto 0);
    variable p    : signed(63 downto 0);
    variable temp_fine_64 : signed(31 downto 0);
    -- TODO: fix pressure raw being std_logic but expecting to be S32
begin
  pressure_raw := resize(signed(pressure_raw_std), pressure_raw'length);
  temp_fine_64 := resize(t_fine, temp_fine_64'length);

  var1 := temp_fine_64 - to_signed(128000, var1'length);
  var2 := resize(var1 * var1 * dig_p6, var2'length);
  var2 := resize(var2 + shift_left(var1 * dig_p5, 17), var2'length);
  var2 := var2 + shift_left(dig_p4, 35);
  var1 := resize(shift_right(var1 * var1 * dig_p3, 8) + shift_left(var1 * dig_p2, 12), var1'length);
  var1 := resize(shift_right((shift_left(to_signed(1, 64), 47) + var1) * dig_p1, 33), var1'length);

  if (var1 /= 0) then
    p := resize(1048576 - pressure_raw, p'length);
    -- Try resizing downto lower size
     p := resize(resize(shift_right((shift_left(p, 31) - var2) * 3125, 32), 32) / resize(shift_right(var1, 32),32), p'length);
    --p := resize(resize(((shift_left(p, 31) - var2) * 3125), 48) / resize(var1,48), p'length);
    var1 := resize(shift_right(dig_p9 * shift_right(p, 13) * shift_right(p, 13), 25), var1'length);
    var2 := resize(shift_right(dig_p8 * p, 19), var2'length);
    p := resize(shift_right(p + var1 + var2, 8) + shift_left(dig_p7, 4), p'length);
    pressure_actual := resize(unsigned(p), pressure_actual'length);
  else 
	-- TODO: handle error reporting
    -- calibration_err <= true;
    pressure_actual := to_unsigned(1, pressure_actual'length);
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
function HumidRawToActual(humid_raw_std : std_logic_vector(humid_byte_width * 8 -1 downto 0);
                          dig_h1 : signed(calibration_byte_width * 8 - 1 downto 0);
                          dig_h2 : signed(calibration_byte_width * 8 - 1 downto 0);
                          dig_h3 : signed(calibration_byte_width * 8 - 1 downto 0);
                          dig_h4 : signed(calibration_byte_width * 8 - 1 downto 0);
                          dig_h5 : signed(calibration_byte_width * 8 - 1 downto 0);
                          dig_h6 : signed(calibration_byte_width * 8 - 1 downto 0);
                          t_fine     : signed(31 downto 0)
                          )
                          return unsigned is
    variable humid_actual : unsigned (31 downto 0);
    variable humid_raw    : signed(31 downto 0);
    variable temp         : signed(31 downto 0);
    variable v_x1_u32r    : signed(31 downto 0);
begin

  humid_raw := resize(signed(humid_raw_std), humid_raw'length);
  v_x1_u32r := t_fine - to_signed(76800, t_fine'length);
  
  v_x1_u32r := resize(shift_right(shift_left(humid_raw, 14) - shift_left(dig_h4, 20) - dig_h5 * v_x1_u32r +
               to_signed(16384, 32), 15) * (
                shift_right( 
                  (
                    shift_right(
                      shift_right( v_x1_u32r * dig_h6, 10) * (shift_right( v_x1_u32r * dig_h3, 11)
                      + to_signed(32768, 32)), 10)
                    + to_signed(2097152, 32))
                  * dig_h2 + to_signed(8192, 32), 14)
               ), v_x1_u32r'length);

  v_x1_u32r := resize((v_x1_u32r - shift_right(
                                        shift_right(
                                                    shift_right(v_x1_u32r, 15) * shift_right(v_x1_u32r, 15)
                                                    , 7) * dig_h1
                                        , 4)
               ), v_x1_u32r'length);
  if v_x1_u32r < 0 then
    v_x1_u32r := to_signed(0, v_x1_u32r'length);
  elsif v_x1_u32r > 419430400 then
    v_x1_u32r :=  to_signed(419430400, v_x1_u32r'length);
  end if;
  
  humid_actual := unsigned(shift_right(v_x1_u32r, 12));
  return humid_actual;
end function;

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
  variable actual_cal    : signed(cal_reg_byte_width * 8 - 1 downto 0);
  variable is_12bit      : boolean;
begin
	 for cal_ind in cal_reg_length - 1 downto 0 loop
		temp_cal_rec := cal_decode_vals(cal_ind);
		
		if(temp_cal_rec.two_bytes) then 
			raw_cal := cal_raw_regs(temp_cal_rec.offset + 1) & cal_raw_regs( temp_cal_rec.offset);
			
      offset_length := temp_cal_rec.bit_offsets(0) - temp_cal_rec.bit_offsets(1);
      offset_length_2 :=  temp_cal_rec.bit_offsets(2) - temp_cal_rec.bit_offsets(3)  + offset_length + 1;
			-- TODO: verify this is correct
			cal_sorted := (others => '0');
			cal_sorted(offset_length downto 0) := raw_cal(temp_cal_rec.bit_offsets(0) downto temp_cal_rec.bit_offsets(1));
			cal_sorted(offset_length_2 downto offset_length + 1) := raw_cal(temp_cal_rec.bit_offsets(2) downto temp_cal_rec.bit_offsets(3));
			
			if temp_cal_rec.is_signed then
				is_12bit := temp_cal_rec.bit_offsets(0) = 11 or temp_cal_rec.bit_offsets(2) = 11;
				if is_12bit then
				  actual_cal := resize(signed(cal_sorted(11 downto 0)), cal_reg_byte_width * 8);
				else
				  actual_cal := resize(signed(cal_sorted), cal_reg_byte_width * 8);
				end if;
			else
			  actual_cal := signed(resize(unsigned(cal_sorted), cal_reg_byte_width * 8));
			end if;
		else -- cal is a single byte
		  if temp_cal_rec.is_signed then
		    actual_cal := resize(signed(cal_raw_regs(temp_cal_rec.offset)), cal_reg_byte_width * 8);
		  else
		    actual_cal := signed(resize(unsigned(cal_raw_regs(temp_cal_rec.offset)), cal_reg_byte_width * 8));
		  end if;
		end if;
		actual_cal_regs(cal_ind) := actual_cal;
	end loop;
	
	return actual_cal_regs;
end function;

end package body FE_CPLD_BME280_Compensate_Data;