-------------------------------------------------------------------------------------
--
--! @file       ad1939_tb.vhd
--! @brief      ad1939 test bench
--! @author     Trevor Vannoy
--! @date       Sept 2020
--! @copyright  Copyright (C) 2020 AudioLogic
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
-- Trevor Vannoy
-- AudioLogic
-- 985 Technology Blvd
-- Bozeman, MT 59718
-- openspeech@flatearthinc.com
--
------------------------------------------------------------------------------------------
library ieee;                 
use ieee.std_logic_1164.all;  
use ieee.numeric_std.all;

use work.ad1939.all;

entity ad1939_tb is
end ad1939_tb;
  
architecture ad1939_tb_arch of ad1939_tb is
    
  component ad1939_hps_audio_mini is
    port (
      sys_clk   : in    std_logic;
      sys_reset : in    std_logic;
      ad1939_adc_asdata2 : in    std_logic; 
      ad1939_adc_abclk   : in    std_logic; 
      ad1939_adc_alrclk  : in    std_logic; 
      ad1939_dac_dsdata1 : out   std_logic; 
      ad1939_dac_dbclk   : out   std_logic; 
      ad1939_dac_dlrclk  : out   std_logic; 
      ad1939_adc_data    : out   std_logic_vector(word_length - 1  downto 0); 
      ad1939_adc_channel : out   std_logic;
      ad1939_adc_valid   : out   std_logic;
      ad1939_dac_data    : in    std_logic_vector(word_length - 1 downto 0);
      ad1939_dac_channel : in    std_logic;
      ad1939_dac_valid   : in    std_logic 
    );
  end component ad1939_hps_audio_mini;

  function create_test_data return std_logic_vector is
    variable test_vector : unsigned(word_length - 1 downto 0) := x"111111";
    variable test_data : std_logic_vector(32 * 8 - 1 downto 0);
  begin
    for i in 1 to 8 loop
      test_data(32 * i - 1 downto 32 * (i - 1)) := '0' & std_logic_vector(test_vector) & "0000000";
      test_vector := test_vector + x"111111";
    end loop;
    return test_data;
  end function create_test_data;

  -- the actual sample frequency is 48 kHz, but that doesn't come out to a nice
  -- number in nanoseconds; the system clock frequency is 2048 times faster 
  -- than the sampling frequency when we run the system clock at 98.304 MHz
  constant sys_clk_period : time := 1 ns;
  constant sample_period : time := 2048 * sys_clk_period;
  constant bclk_period : time := sample_period/64;
      
  signal sys_clk : std_logic;
  signal sys_reset : std_logic;
  signal lrclk : std_logic;
  signal bclk : std_logic;

  signal avalon_data : std_logic_vector(word_length - 1 downto 0);
  signal avalon_channel : std_logic := '0';
  signal avalon_valid : std_logic := '0';
  
  signal serial_data_in : std_logic := '0';
  signal serial_data_out : std_logic := '0';

  signal test_data : std_logic_vector(32 * 8 - 1 downto 0);

  signal done : boolean := false;
begin

  ad1939_audio_mini: ad1939_hps_audio_mini 
    port map (
      sys_clk => sys_clk,
      sys_reset => sys_reset,
      ad1939_adc_asdata2 => serial_data_in,
      ad1939_adc_abclk => bclk,
      ad1939_adc_alrclk => lrclk,
      ad1939_dac_dsdata1 => serial_data_out,
      ad1939_dac_dbclk => open,
      ad1939_dac_dlrclk => open,
      ad1939_adc_data => avalon_data,
      ad1939_adc_channel => avalon_channel,
      ad1939_adc_valid => avalon_valid,
      ad1939_dac_data => avalon_data,
      ad1939_dac_channel => avalon_channel,
      ad1939_dac_valid => avalon_valid
    );

  sys_reset <= '0';
  
  test_data <= create_test_data;

  generate_sys_clk: process
  begin
    if not done then
      sys_clk <= '0';
      wait for sys_clk_period/2;
      sys_clk <= '1';
      wait for sys_clk_period/2;
    else
      wait;
    end if;
  end process;

  generate_lrclk: process
  begin
    if not done then
      lrclk <= '0';
      wait for sample_period/2;
      lrclk <= '1';
      wait for sample_period/2;
    else 
      wait;
    end if;
  end process;

  generate_bclk: process
  begin
    if not done then
      bclk <= '0';
      wait for bclk_period/2;
      bclk <= '1';
      wait for bclk_period/2;
    else
      wait;
    end if;
  end process;

  -- data comes in MSB first from the codec, so I start at the
  -- leftmost index and substract to simulate this. 
  -- stream_data: process(bclk)
  --   variable data_index : natural := test_data'left;
  -- begin
  --   if (rising_edge(bclk)) then
  --     serial_data_in <= test_data(data_index);
  --     data_index := data_index - 1;
  --   end if;
  -- end process;

  stream_data: process
    variable data_index : natural := test_data'left;
  begin
    for i in test_data'left downto 0 loop
      wait until rising_edge(bclk);
      serial_data_in <= test_data(i);
    end loop;
    done <= true;
    wait;
  end process;

end architecture;
            
            