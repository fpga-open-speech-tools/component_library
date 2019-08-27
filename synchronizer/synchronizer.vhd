-------------------------------------------------------------------------------------
--
--! @file       syncrhonizer.vhd
--! @brief      A 2-stage synchronizer
--! @details    Synchronizes input data to a desired clock domain
--!             with a 2-stage shift register
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
--! @brief      synchronizer
--! @details    Synchronizes din to clk using a 2-stage shift register;
--!             output latency of 2 clock cycles
--! @param      clk   Clock domain to synchroniz data with
--! @param      rst   Asynchronous active-high reset
--! @param      din   Input data to synchronize
--! @param      dout  Synchronized output data
--
----------------------------------------------------------------------------
entity synchronizer is
	generic (
		data_width : integer := 1
	);
	port (
		clk   : in  std_logic;
		rst   : in  std_logic;
		din   : in  std_logic_vector(data_width - 1 downto 0);
		dout  : out std_logic_vector(data_width - 1 downto 0)
	);
end entity;

architecture syncrhonizer_arch of synchronizer is

  ------------------------------------------------------------------------------------------
  -- Constant and Type Declarations
  ------------------------------------------------------------------------------------------
	type delay_line is array (1 downto 0) of std_logic_vector(data_width - 1 downto 0);

  ------------------------------------------------------------------------------------------
  -- Signal Declarations
  ------------------------------------------------------------------------------------------
	signal shift_reg : delay_line := ((others => '0'), (others => '0'));

begin
	-- run data through a 2-stage shift register
	sync : process(clk, rst)
	begin
		if rst = '1' then
			shift_reg <= ((others => '0'), (others => '0'));
		elsif rising_edge(clk) then
			shift_reg <= shift_reg(0) & din;
		end if;
	end process;

	-- take the output at the second stage
	dout <= shift_reg(1);

end architecture;
