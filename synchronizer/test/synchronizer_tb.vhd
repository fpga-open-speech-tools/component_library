-------------------------------------------------------------------------------------
--
--! @file       synchronizer_tb.vhd
--! @brief      synchronizer test bench
--! @details    test bench for synchronizer component
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
--! @brief      synchronizer test bench
--! @details    test bench for synchronizer component
--
----------------------------------------------------------------------------
entity synchronizer_tb is
end synchronizer_tb;

architecture synchronizer_tb_arch of synchronizer_tb is

    component synchronizer
        generic (
            data_width : integer := 1
        );
        port (
            clk     : in std_logic;
            rst     : in std_logic;
            din     : in std_logic_vector(data_width - 1 downto 0);
            dout    : out std_logic_vector(data_width - 1 downto 0)
        );
    end component;

    constant data_width : integer := 32;

    signal clk : std_logic;
    signal rst : std_logic;
    signal din : std_logic_vector(data_width - 1 downto 0);
    signal dout : std_logic_vector(data_width - 1 downto 0);

begin

    synchronizer_0: synchronizer
        generic map (
            data_width => data_width
        )
        port map (
            clk     => clk,
            rst     => rst,
            din     => din,
            dout    => dout
        );

    rst <= '0';

    -- simulation clock
    process
    begin
        clk <= '0';
        wait for 20 ns;
        clk <= '1';
        wait for 20 ns;
    end process;

    -- simulation data pattern
    process
    begin
        din <= 32x"0";
        wait for 5 ns;
        din <= 32x"beef";
        wait for 80 ns;
        din <= 32x"5a5abc";
        wait;
    end process;

end architecture;
