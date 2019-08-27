-------------------------------------------------------------------------------------
--
--! @file       tapped_delay_line.vhd
--! @brief      Generic tapped delay line
--! @details    Arbitrary-length delay line with a mux to select a bit from any stage
--! @author     Trevor Vannoy
--! @date       September 2018
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
--! @brief    tapped_delay_line
--! @details  Arbitrary-length delay line with a mux to select a bit from any stage
--! @param    clk   Input clock
--! @param    rst   Asynchronous active-high reset
--! @param    din   Input data bit
--! @param    sel   Mux select line
--! @param    dout  Output data bit selected by mux
--
----------------------------------------------------------------------------
entity tapped_delay_line is
  generic (
    length : integer := 16
  );
  port (
    clk   : in  std_logic;
    rst   : in  std_logic;
    din   : in  std_logic;
    sel   : in  integer range 0 to length - 1;
    dout  : out std_logic := '0'
  );
end entity;

architecture tapped_delay_line_arch of tapped_delay_line is

  ------------------------------------------------------------------------------------------
  -- Constant and Type Declarations
  ------------------------------------------------------------------------------------------

  ------------------------------------------------------------------------------------------
  -- Signal Declarations
  ------------------------------------------------------------------------------------------
  signal dout_vec : std_logic_vector(length - 1 downto 0) := (others => '0');

begin
  -- shift in the serial data
  shift_register : process(clk, rst)
  begin
    if rst = '1' then
      dout_vec <= (others => '0');
    elsif rising_edge(clk) then
      -- shift left so the new bit gets shifted into lsb
      dout_vec <= dout_vec(length - 2 downto 0) & din;
    end if;
  end process;

  -- multiplexer
  dout <= dout_vec(sel);

end architecture;
