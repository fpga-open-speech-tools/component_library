-------------------------------------------------------------------------------------
--
--! @file       frame_start_counter.vhd
--! @brief      Indicate start of data frame
--! @details    Indicates the start of a TDM data frame with a pulse delayed
--!             by a specified number of clock cycles
--! @author     Trevor Vannoy
--! @date       August 2018
--! @copyright  Copyright (C) 2018 Trevor Vannoy
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

library ieee;                  --! Use standard library.
use ieee.std_logic_1164.all;   --! Use standard logic elements.
use ieee.numeric_std.all;      --! Use numeric standard.

----------------------------------------------------------------------------
--
--! @brief    frame_start_counter
--! @details  Indicates the start of a TDM data frame with a pulse delayed
--!           by frame_delay clock cycles
--! @param    sck           Phase-shifted sampling clock for incoming data
--! @param    rst           Asynchronous active-high reset
--! @param    frame_delay   Number of clock cycles the received data
--!                         frame is delayed by relative to the original
--!                         sampling pulse
--! @param    frame_start   Pulse indicating start of data frame
--
----------------------------------------------------------------------------
-- TODO: better entity name?
entity frame_start_counter is
  port (
    sck         : in  std_logic;
    rst         : in  std_logic;
    frame_delay : in  integer;
    frame_start : out std_logic
  );
end entity;

architecture frame_start_counter_arch of frame_start_counter is
  ------------------------------------------------------------------------------------------
  -- Constant and Type Declarations
  ------------------------------------------------------------------------------------------
    constant frame_length   : integer := 512;

  ------------------------------------------------------------------------------------------
  -- Signal Declarations
  ------------------------------------------------------------------------------------------
    signal count            : integer := 0;

begin

  -- wait frame_delay cycles then output the frame_start pulse for one clock cycle
  generate_frame_start_pulse : process(sck, rst)
  begin
    if rst = '1' then
      count <= 0;
      frame_start <= '0';
    elsif rising_edge(sck) then
      if count < frame_length - 1 then
        count <= count + 1;
      else
        count <= 0;
      end if;
      if count = frame_delay then
        frame_start <= '1';
      else
        frame_start <= '0';
      end if;
    end if;
  end process;

end architecture;
