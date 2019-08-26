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
  signal data_in : std_logic_vector(2*length - 1 downto 0) := x"ABCDEF53";
  signal expected_data_out0 : std_logic_vector(length - 1 downto 0) := data_in(15 downto 0);
  signal expected_data_out1 : std_logic_vector(length - 1 downto 0) := data_in(16 downto 1);
  signal expected_data_out2 : std_logic_vector(length - 1 downto 0) := data_in(17 downto 2);
  signal expected_data_out3 : std_logic_vector(length - 1 downto 0) := data_in(18 downto 3);
  signal expected_data_out4 : std_logic_vector(length - 1 downto 0) := data_in(19 downto 4);
  signal expected_data_out5 : std_logic_vector(length - 1 downto 0) := data_in(20 downto 5);
  signal expected_data_out6 : std_logic_vector(length - 1 downto 0) := data_in(21 downto 6);
  signal expected_data_out7 : std_logic_vector(length - 1 downto 0) := data_in(22 downto 7);
  signal expected_data_out8 : std_logic_vector(length - 1 downto 0) := data_in(23 downto 8);
  signal expected_data_out9 : std_logic_vector(length - 1 downto 0) := data_in(24 downto 9);
  signal expected_data_out10 : std_logic_vector(length - 1 downto 0) := data_in(25 downto 10);
  signal expected_data_out11 : std_logic_vector(length - 1 downto 0) := data_in(26 downto 11);
  signal expected_data_out12 : std_logic_vector(length - 1 downto 0) := data_in(27 downto 12);
  signal expected_data_out13 : std_logic_vector(length - 1 downto 0) := data_in(28 downto 13);
  signal expected_data_out14 : std_logic_vector(length - 1 downto 0) := data_in(29 downto 14);
  signal expected_data_out15 : std_logic_vector(length - 1 downto 0) := data_in(30 downto 15);

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
  signal data_out0 : std_logic_vector(length-1 downto 0) := (others => '0');
  signal data_out1 : std_logic_vector(length-1 downto 0) := (others => '0');
  signal data_out2 : std_logic_vector(length-1 downto 0) := (others => '0');
  signal data_out3 : std_logic_vector(length-1 downto 0) := (others => '0');
  signal data_out4 : std_logic_vector(length-1 downto 0) := (others => '0');
  signal data_out5 : std_logic_vector(length-1 downto 0) := (others => '0');
  signal data_out6 : std_logic_vector(length-1 downto 0) := (others => '0');
  signal data_out7 : std_logic_vector(length-1 downto 0) := (others => '0');
  signal data_out8 : std_logic_vector(length-1 downto 0) := (others => '0');
  signal data_out9 : std_logic_vector(length-1 downto 0) := (others => '0');
  signal data_out10 : std_logic_vector(length-1 downto 0) := (others => '0');
  signal data_out11 : std_logic_vector(length-1 downto 0) := (others => '0');
  signal data_out12 : std_logic_vector(length-1 downto 0) := (others => '0');
  signal data_out13 : std_logic_vector(length-1 downto 0) := (others => '0');
  signal data_out14 : std_logic_vector(length-1 downto 0) := (others => '0');
  signal data_out15 : std_logic_vector(length-1 downto 0) := (others => '0');
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
    for i in 0 to 32 loop
      clk <= '0';
      wait for 20 ns;
      clk <= '1';
      k <= k + 1;
      din <= data_in(data_in'length - 1);
      data_in <= data_in(data_in'length - 2 downto 0) & '0';
      data_out0 <= data_out0(14 downto 0) & dout0;
      data_out1 <= data_out1(14 downto 0) & dout1;
      data_out2 <= data_out2(14 downto 0) & dout2;
      data_out3 <= data_out3(14 downto 0) & dout3;
      data_out4 <= data_out4(14 downto 0) & dout4;
      data_out5 <= data_out5(14 downto 0) & dout5;
      data_out6 <= data_out6(14 downto 0) & dout6;
      data_out7 <= data_out7(14 downto 0) & dout7;
      data_out8 <= data_out8(14 downto 0) & dout8;
      data_out9 <= data_out9(14 downto 0) & dout9;
      data_out10 <= data_out10(14 downto 0) & dout10;
      data_out11 <= data_out11(14 downto 0) & dout11;
      data_out12 <= data_out12(14 downto 0) & dout12;
      data_out13 <= data_out13(14 downto 0) & dout13;
      data_out14 <= data_out14(14 downto 0) & dout14;
      data_out15 <= data_out15(14 downto 0) & dout15;
      wait for 20 ns;
    end loop;
  end process;


  -- process
  -- begin
  --   wait until rising_edge(clk);
  -- end process;

  process
  begin
    wait for 32*40 ns;
    assert (data_out0 /= expected_data_out0) report "Failed test for data_out_0" severity FAILURE;
    assert (data_out1 /= expected_data_out1) report "Failed test for data_out_1" severity FAILURE;
    assert (data_out2 /= expected_data_out2) report "Failed test for data_out_2" severity FAILURE;
    assert (data_out3 /= expected_data_out3) report "Failed test for data_out_3" severity FAILURE;
    assert (data_out4 /= expected_data_out4) report "Failed test for data_out_4" severity FAILURE;
    assert (data_out5 /= expected_data_out5) report "Failed test for data_out_5" severity FAILURE;
    assert (data_out6 /= expected_data_out6) report "Failed test for data_out_6" severity FAILURE;
    assert (data_out7 /= expected_data_out7) report "Failed test for data_out_7" severity FAILURE;
    assert (data_out8 /= expected_data_out8) report "Failed test for data_out_8" severity FAILURE;
    assert (data_out9 /= expected_data_out9) report "Failed test for data_out_9" severity FAILURE;
    assert (data_out10 /= expected_data_out10) report "Failed test for data_out_10" severity FAILURE;
    assert (data_out11 /= expected_data_out11) report "Failed test for data_out_11" severity FAILURE;
    assert (data_out12 /= expected_data_out12) report "Failed test for data_out_12" severity FAILURE;
    assert (data_out13 /= expected_data_out13) report "Failed test for data_out_13" severity FAILURE;
    assert (data_out14 /= expected_data_out14) report "Failed test for data_out_14" severity FAILURE;
    assert (data_out15 /= expected_data_out15) report "Failed test for data_out_15" severity FAILURE;
    assert (data_out0 = expected_data_out0) report "Passed test for data_out_0" severity NOTE;
    assert (data_out1 = expected_data_out1) report "Passed test for data_out_1" severity NOTE;
    assert (data_out2 = expected_data_out2) report "Passed test for data_out_2" severity NOTE;
    assert (data_out3 = expected_data_out3) report "Passed test for data_out_3" severity NOTE;
    assert (data_out4 = expected_data_out4) report "Passed test for data_out_4" severity NOTE;
    assert (data_out5 = expected_data_out5) report "Passed test for data_out_5" severity NOTE;
    assert (data_out6 = expected_data_out6) report "Passed test for data_out_6" severity NOTE;
    assert (data_out7 = expected_data_out7) report "Passed test for data_out_7" severity NOTE;
    assert (data_out8 = expected_data_out8) report "Passed test for data_out_8" severity NOTE;
    assert (data_out9 = expected_data_out9) report "Passed test for data_out_9" severity NOTE;
    assert (data_out10 = expected_data_out10) report "Passed test for data_out_10" severity NOTE;
    assert (data_out11 = expected_data_out11) report "Passed test for data_out_11" severity NOTE;
    assert (data_out12 = expected_data_out12) report "Passed test for data_out_12" severity NOTE;
    assert (data_out13 = expected_data_out13) report "Passed test for data_out_13" severity NOTE;
    assert (data_out14 = expected_data_out14) report "Passed test for data_out_14" severity NOTE;
    assert (data_out15 = expected_data_out15) report "Passed test for data_out_15" severity NOTE;
  end process;


end architecture;
