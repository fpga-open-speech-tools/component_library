-------------------------------------------------------------------------------------
--
--! @file       mic_array_deserializer.vhd
--! @brief      Deserialze mic array data
--! @details    Deserialize mic array data by using a shift register and some
--!             bit and word counters to separate the TDM channels/words.
--!             The incoming mic array data words are 24-bit, which are
--!             MSB-aligned in a 32-bit slot (i.e. the first 24 bits we received
--!             is the data we are interested in).
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

library ieee;                 --! Use standard library.
library work;                 --! Use local library.
use ieee.std_logic_1164.all;  --! Use standard logic elements.
use ieee.numeric_std.all;     --! Use numeric standard.
use ieee.math_real.all;       --! Use math library for log2
use work.mic_array_param.all; --! Use mic array related constant parameters

----------------------------------------------------------------------------
--
--! @brief    mic_array_deserializer
--! @details  Deserialize mic array data by using a shift register and some
--!           bit and word counters to separate the TDM channels/words.
--! @param    clk           Data sampling clock
--! @param    rst           Asynchronous active-high reset
--! @param    frame_start   Pulse indicating start of data frame
--! @param    din           Serial input data
--! @param    dout          Parallel output data
--! @param    word_clock    Clock at the frequency of the incoming words
--! @param    channel       TDM channel number
--! @param    valid         Flag indicating when output data is valid for
--!                         use with streaming interfaces
--
----------------------------------------------------------------------------
entity mic_array_deserializer is
  port (
    clk         : in  std_logic;
    rst         : in  std_logic;
    frame_start : in  std_logic;
    din         : in  std_logic;
    dout        : out std_logic_vector(data_width-1 downto 0) := (others => '0');
    word_clock  : out std_logic;
    channel     : out std_logic_vector(integer(ceil(log2(real(num_channels))))-1 downto 0);
    valid       : out std_logic := '0'
  );
end entity;

architecture mic_array_deserializer_arch of mic_array_deserializer is

  ------------------------------------------------------------------------------------------
  -- Constant and Type Declarations
  ------------------------------------------------------------------------------------------
  constant num_pad_bits : integer := tdm_slot_width - data_width;

  ------------------------------------------------------------------------------------------
  -- Signal Declarations
  ------------------------------------------------------------------------------------------
  signal dout_tmp    : std_logic_vector(mic_data_width-1 downto 0) := (others => '0');
  signal bit_count    : integer := 0;
  signal channel_temp : integer := -1;
  signal word_done    : boolean := false;

begin
  -- shift in the serial data
  shift_register : process(clk, rst)
  begin
    if rst = '1' then
      dout_tmp <= (others => '0');
    elsif rising_edge(clk) then
      -- shift left so the new bit gets shifted into lsb
      dout_tmp <= dout_tmp(dout_tmp'high-1 downto 0) & din;
    end if;
  end process;

  -- latch the deserialized word once it is all the way into the the shift register
  -- additionally, set valid flag for one clock cycle for use with streaming interfaces
  output_reg : process(clk, rst)
  begin
    if rst = '1' then
      dout <= (others => '0');
    elsif rising_edge(clk) then
      if word_done then
        -- convert the incoming 24-bit signed word into a 32.28 word by adding 4
        -- fractional bits and sign-extending.
        dout(data_fraction-1 downto 0) <= dout_tmp & "0000";
        dout(data_width-1 downto data_fraction) <= (others => dout_tmp(dout_tmp'high));
        valid <= '1';
      else
        valid <= '0';
      end if;
    end if;
  end process;

  -- count the number of incoming bits so we know where the words start/end
  bit_counter : process(clk, rst)
  begin
    if rst = '1' then
      bit_count <= 0;
      word_done <= false;
    elsif rising_edge(clk) then
      -- count bits until we reach the next TDM slot, then restart the count
      if frame_start = '1' then
        bit_count <= 1;
      elsif bit_count < tdm_slot_width - 1 then
        bit_count <= bit_count + 1;
      else
        bit_count <= 0;
      end if;
      -- check if the word is done
      if bit_count = mic_data_width - 1 then
        word_done <= true;
      else
        word_done <= false;
      end if;
    end if;
  end process;

  -- count how many words have come through so we know what channel we're on
  word_counter : process(clk, rst)
  begin
    if rst = '1' then
      channel_temp <= -1;
    elsif rising_edge(clk) and word_done then
      if channel_temp < num_channels - 1 then
        channel_temp <= channel_temp + 1;
      else
        channel_temp <= 0;
      end if;
    end if;
  end process;

  channel <= std_logic_vector(to_unsigned(channel_temp, channel'length));
  word_clock <= '1' when word_done else '0'; -- TODO: remove word_clock signal? I don't really need it now that I've captured data with MATLAB.

end architecture;
