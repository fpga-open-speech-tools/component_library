-------------------------------------------------------------------------------------
--
--! @file       frame_start_counter_tb.vhd
--! @brief      frame_start_counter test bench
--! @details    test bench for frame_start_counter component
--! @author     Trevor Vannoy
--! @date       Aug 2018
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
--! @brief      frame_start_counter test bench
--! @details    test bench for frame_start_counter component
--
----------------------------------------------------------------------------
entity frame_start_counter_tb is
end frame_start_counter_tb;

architecture frame_start_counter_tb_arch of frame_start_counter_tb is

    component frame_start_counter
        port (
            clk             : in std_logic;
            rst             : in std_logic;
            frame_delay     : in integer;
            frame_start     : out std_logic
        );
    end component;


    signal clk          : std_logic;
    signal rst          : std_logic;
    signal delay        : integer;
    signal frame_start  : std_logic;

begin

    frame_start_counter_0: frame_start_counter port map (
        clk         => clk,
        rst         => rst,
        frame_delay => delay,
        frame_start => frame_start);

    rst <= '0';
    delay <= 2;

    process
    begin
        clk <= '0';
        wait for 20 ns;
        clk <= '1';
        wait for 20 ns;
    end process;

end architecture;
