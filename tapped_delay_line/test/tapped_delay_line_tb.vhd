-------------------------------------------------------------------------------------
--
--! @file       tapped_delay_line_tb.vhd
--! @brief      tapped_delay_line test bench
--! @details    test bench for tapped_delay_line component
--! @author     Trevor Vannoy
--! @date       Sept 2018
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
--! @brief      tapped_delay_line test bench
--! @details    test bench for tapped_delay_line component
--
----------------------------------------------------------------------------
entity tapped_delay_line_tb is
end tapped_delay_line_tb;

architecture tapped_delay_line_tb_arch of tapped_delay_line_tb is

  component tapped_delay_line
    generic (
      length : integer := 16
    );
    port (
      clk     : in std_logic;
      rst     : in std_logic;
      din     : in std_logic;
      sel     : in integer range 0 to length - 1;
      dout    : out std_logic := '0'
    );
  end component;

  constant length : integer := 16;
  signal data_in : std_logic_vector(length - 1 downto 0) := x"ABCD";
  signal expected_data_out0  : std_logic := data_in(0);
  signal expected_data_out1  : std_logic := data_in(1);
  signal expected_data_out2  : std_logic := data_in(2);
  signal expected_data_out3  : std_logic := data_in(3);
  signal expected_data_out4  : std_logic := data_in(4);
  signal expected_data_out5  : std_logic := data_in(5);
  signal expected_data_out6  : std_logic := data_in(6);
  signal expected_data_out7  : std_logic := data_in(7);
  signal expected_data_out8  : std_logic := data_in(8);
  signal expected_data_out9  : std_logic := data_in(9);
  signal expected_data_out10 : std_logic := data_in(10);
  signal expected_data_out11 : std_logic := data_in(11);
  signal expected_data_out12 : std_logic := data_in(12);
  signal expected_data_out13 : std_logic := data_in(13);
  signal expected_data_out14 : std_logic := data_in(14);
  signal expected_data_out15 : std_logic := data_in(15);

  signal clk : std_logic;
  signal rst : std_logic;
  signal din : std_logic := '0';
  signal dout0 : std_logic := '0';
  signal dout1 : std_logic := '0';
  signal dout2 : std_logic := '0';
  signal dout3 : std_logic := '0';
  signal dout4 : std_logic := '0';
  signal dout5 : std_logic := '0';
  signal dout6 : std_logic := '0';
  signal dout7 : std_logic := '0';
  signal dout8 : std_logic := '0';
  signal dout9 : std_logic := '0';
  signal dout10 : std_logic := '0';
  signal dout11 : std_logic := '0';
  signal dout12 : std_logic := '0';
  signal dout13 : std_logic := '0';
  signal dout14 : std_logic := '0';
  signal dout15 : std_logic := '0';
  signal k : unsigned(length - 1 downto 0) := (others => '0');

begin

  tapped_delay_line_0: tapped_delay_line
    generic map (
      length => length
    )
    port map (
      clk     => clk,
      rst     => rst,
      din     => din,
      sel     => 0,
      dout    => dout0
    );

  tapped_delay_line_1: tapped_delay_line
    generic map (
      length => length
    )
    port map (
      clk     => clk,
      rst     => rst,
      din     => din,
      sel     => 1,
      dout    => dout1
    );

  tapped_delay_line_2: tapped_delay_line
    generic map (
      length => length
    )
    port map (
      clk     => clk,
      rst     => rst,
      din     => din,
      sel     => 2,
      dout    => dout2
    );

  tapped_delay_line_3: tapped_delay_line
    generic map (
      length => length
    )
    port map (
      clk     => clk,
      rst     => rst,
      din     => din,
      sel     => 3,
      dout    => dout3
    );

  tapped_delay_line_4: tapped_delay_line
    generic map (
      length => length
    )
    port map (
      clk     => clk,
      rst     => rst,
      din     => din,
      sel     => 4,
      dout    => dout4
    );

  tapped_delay_line_5: tapped_delay_line
    generic map (
      length => length
    )
    port map (
      clk     => clk,
      rst     => rst,
      din     => din,
      sel     => 5,
      dout    => dout5
    );

  tapped_delay_line_6: tapped_delay_line
    generic map (
      length => length
    )
    port map (
      clk     => clk,
      rst     => rst,
      din     => din,
      sel     => 6,
      dout    => dout6
    );

  tapped_delay_line_7: tapped_delay_line
    generic map (
      length => length
    )
    port map (
      clk     => clk,
      rst     => rst,
      din     => din,
      sel     => 7,
      dout    => dout7
    );

  tapped_delay_line_8: tapped_delay_line
    generic map (
      length => length
    )
    port map (
      clk     => clk,
      rst     => rst,
      din     => din,
      sel     => 8,
      dout    => dout8
    );

  tapped_delay_line_9: tapped_delay_line
    generic map (
      length => length
    )
    port map (
      clk     => clk,
      rst     => rst,
      din     => din,
      sel     => 9,
      dout    => dout9
    );

  tapped_delay_line_10: tapped_delay_line
    generic map (
      length => length
    )
    port map (
      clk     => clk,
      rst     => rst,
      din     => din,
      sel     => 10,
      dout    => dout10
    );

  tapped_delay_line_11: tapped_delay_line
    generic map (
      length => length
    )
    port map (
      clk     => clk,
      rst     => rst,
      din     => din,
      sel     => 11,
      dout    => dout11
    );

  tapped_delay_line_12: tapped_delay_line
    generic map (
      length => length
    )
    port map (
      clk     => clk,
      rst     => rst,
      din     => din,
      sel     => 12,
      dout    => dout12
    );

  tapped_delay_line_13: tapped_delay_line
    generic map (
      length => length
    )
    port map (
      clk     => clk,
      rst     => rst,
      din     => din,
      sel     => 13,
      dout    => dout13
    );

  tapped_delay_line_14: tapped_delay_line
    generic map (
      length => length
    )
    port map (
      clk     => clk,
      rst     => rst,
      din     => din,
      sel     => 14,
      dout    => dout14
    );

  tapped_delay_line_15: tapped_delay_line
    generic map (
      length => length
    )
    port map (
      clk     => clk,
      rst     => rst,
      din     => din,
      sel     => 15,
      dout    => dout15
    );

  rst <= '0';

  -- simulation clock
  process
  begin
    for i in 0 to 15 loop
      clk <= '0';
      wait for 20 ns;
      clk <= '1';
      din <= data_in(data_in'length - 1);
      data_in <= data_in(data_in'length - 2 downto 0) & '0';
      wait for 20 ns;
    end loop;
  end process;


  -- process
  -- begin
  --   wait until rising_edge(clk);
  -- end process;

  process
  begin
    wait for 16*40 ns;
    -- XXX: these assertions are failing in GHDL, but the output waveforms in gtkwave are correct
    -- XXX: so the hardware appears to be working but the testbench is reporting otherwise....
    assert (dout0 /= expected_data_out0) report "Failed test for data_out_0" severity FAILURE;
    assert (dout1 /= expected_data_out1) report "Failed test for data_out_1" severity FAILURE;
    assert (dout2 /= expected_data_out2) report "Failed test for data_out_2" severity FAILURE;
    assert (dout3 /= expected_data_out3) report "Failed test for data_out_3" severity FAILURE;
    assert (dout4 /= expected_data_out4) report "Failed test for data_out_4" severity FAILURE;
    assert (dout5 /= expected_data_out5) report "Failed test for data_out_5" severity FAILURE;
    assert (dout6 /= expected_data_out6) report "Failed test for data_out_6" severity FAILURE;
    assert (dout7 /= expected_data_out7) report "Failed test for data_out_7" severity FAILURE;
    assert (dout8 /= expected_data_out8) report "Failed test for data_out_8" severity FAILURE;
    assert (dout9 /= expected_data_out9) report "Failed test for data_out_9" severity FAILURE;
    assert (dout10 /= expected_data_out10) report "Failed test for data_out_10" severity FAILURE;
    assert (dout11 /= expected_data_out11) report "Failed test for data_out_11" severity FAILURE;
    assert (dout12 /= expected_data_out12) report "Failed test for data_out_12" severity FAILURE;
    assert (dout13 /= expected_data_out13) report "Failed test for data_out_13" severity FAILURE;
    assert (dout14 /= expected_data_out14) report "Failed test for data_out_14" severity FAILURE;
    assert (dout15 /= expected_data_out15) report "Failed test for data_out_15" severity FAILURE;
    assert (dout0 = expected_data_out0) report "Passed test for data_out_0" severity NOTE;
    assert (dout1 = expected_data_out1) report "Passed test for data_out_1" severity NOTE;
    assert (dout2 = expected_data_out2) report "Passed test for data_out_2" severity NOTE;
    assert (dout3 = expected_data_out3) report "Passed test for data_out_3" severity NOTE;
    assert (dout4 = expected_data_out4) report "Passed test for data_out_4" severity NOTE;
    assert (dout5 = expected_data_out5) report "Passed test for data_out_5" severity NOTE;
    assert (dout6 = expected_data_out6) report "Passed test for data_out_6" severity NOTE;
    assert (dout7 = expected_data_out7) report "Passed test for data_out_7" severity NOTE;
    assert (dout8 = expected_data_out8) report "Passed test for data_out_8" severity NOTE;
    assert (dout9 = expected_data_out9) report "Passed test for data_out_9" severity NOTE;
    assert (dout10 = expected_data_out10) report "Passed test for data_out_10" severity NOTE;
    assert (dout11 = expected_data_out11) report "Passed test for data_out_11" severity NOTE;
    assert (dout12 = expected_data_out12) report "Passed test for data_out_12" severity NOTE;
    assert (dout13 = expected_data_out13) report "Passed test for data_out_13" severity NOTE;
    assert (dout14 = expected_data_out14) report "Passed test for data_out_14" severity NOTE;
    assert (dout15 = expected_data_out15) report "Passed test for data_out_15" severity NOTE;
  end process;


end architecture;
