-------------------------------------------------------------------------------------
--
--! @file       mic_array.vhd
--! @brief      Mic array constants
--! @details    Miscellaneous constants/parameters related to the microphone arrays
--! @author     Trevor Vannoy
--! @date       January 2019
--! @copyright  Copyright (C) 2019 Trevor Vannoy
--!
--! Software Released Under the MIT License
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
--  Trevor Vannoy
--  Electrical and Computer Engineering
--  Montana State University
--  610 Cobleigh Hall
--  Bozeman, MT 59717
--
------------------------------------------------------------------------------------------
library ieee;                 --! Use standard library.
use ieee.math_real.all;       --! Use math library for log2

package mic_array_param is

  ------------------------------------------------------------------------------------------
  -- Constant and Type Declarations
  ------------------------------------------------------------------------------------------
  --! the width of the 24-bit word being sent from the microphones
  constant mic_data_width     : integer := 24;
  --! the width of the TDM slot in the microphone array data stream
  constant tdm_slot_width     : integer := 32;

  --! width of the output data. We make it 32.28 to normalize the incoming 24-bit data to
  --! +/- 1 and allow for a little bit of overflow.
  constant data_width         : integer := 32;
  --! number of fractional bits in the output data
  constant data_fraction      : integer := 28;
  --! number of integer bits in the output data
  constant data_integer       : integer := 4;

  --! number of channel in the data stream; also the number of microphones in the array
  constant num_channels       : integer := 16;
  constant ch_width           : integer := integer(ceil(log2(real(num_channels))));

  --! length of the delay line used to synchronize incoming data between microphone arrays
  constant delay_line_length  : integer := 16;

end package;
