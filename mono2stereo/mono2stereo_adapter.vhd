-------------------------------------------------------------------------------------
--
--! @file       mono2stereo_adapter.vhd
--! @brief      Convert a stream of single channel data to two-channel data.
--! @details    Given a stream of signle channel data (e.g. mono audio) in
--!             channel-data-valid format, convert the data into two-channel
--!             data (e.g. stereo audio). This is done by changing the
--!             channel number and asserting the valid pulse every T/2,
--!             where T is number of clock cycles per sampling period.
--! @author     Trevor Vannoy
--! @date       March 2019
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
use ieee.std_logic_1164.all;  --! Use standard logic elements.
use ieee.numeric_std.all;     --! Use numeric standard.

----------------------------------------------------------------------------
--
--! @brief    mono2stereo_adapter
--! @details  Given a stream of single channel data (e.g. mono audio) in
--!           channel-data-valid format, convert the data into two-channel
--!           data (e.g. stereo audio). This is done by changing the
--!           channel number and asserting the valid pulse every T/2,
--!           where T is number of clock cycles per sampling period.
--! @param    clk           system clock
--! @param    rst           system reset
--! @param    data_in       incoming TDM mono data stream
--! @param    channel_in    incoming TDM channel number
--! @param    valid_in      valid flag asserted when incoming data is valid
--! @param    data_out      outgoing TDM stereo data stream
--! @param    channel_out   outgoing TDM channel number
--! @param    valid_out     valid flag asserted when outgoing data is valid
--
----------------------------------------------------------------------------
entity mono2stereo_adapter is
  generic (
    sample_rate : natural := 48000;
    clk_freq    : natural := 24576000;
    data_width  : natural := 32
  );
  port (
    clk         : in  std_logic;
    rst         : in  std_logic;
    data_in     : in  std_logic_vector(data_width-1 downto 0);
    channel_in  : in  std_logic;
    valid_in    : in  std_logic;
    data_out    : out std_logic_vector(data_width-1 downto 0) := (others => '0');
    channel_out : out std_logic := '0';
    valid_out   : out std_logic := '0'
  );
end entity;

architecture mono2stereo_adapter_arch of mono2stereo_adapter is

  ------------------------------------------------------------------------------------------
  -- Constant and Type Declarations
  ------------------------------------------------------------------------------------------
  constant cycles_per_sample : natural := clk_freq/sample_rate;
  -- attribute keep : boolean;
  -- attribute keep of cycles_per_sample: constant is true;
  -- signal cnt1 : integer;
  -- signal cnt2 : integer;

  ------------------------------------------------------------------------------------------
  -- Signal Declarations
  ------------------------------------------------------------------------------------------

begin

  channel_generator : process(clk)
    variable i : natural range 0 to cycles_per_sample - 1 := 0;
  begin
    if rising_edge(clk) then
      -- reset the counter when new valid data comes in; this ensures that the counter
      -- starts counting at the beginning of each frame
      if valid_in = '1' then
        i := 0;
      else
        -- alternate the channel number between 0 and 1 every half period
        if i < cycles_per_sample/2 then
          i := i + 1;
          channel_out <= '0';
        elsif i < cycles_per_sample-1 then
          i := i + 1;
          channel_out <= '1';
        else
          i := 0;
        end if;
      end if;
    end if;
  end process;

  valid_generator : process(clk)
    variable i : natural range 0 to cycles_per_sample - 1 := 0;
  begin
    if rising_edge(clk) then
      -- reset the counter when new valid data comes in; this ensures that the counter
      -- starts counting at the beginning of each frame
      if valid_in = '1' then
        i := 0;
      else
        -- assert valid signal for 1 clock cycle every half period
        if i = 0 or i = cycles_per_sample/2 then
          i := i + 1;
          valid_out <= '1';
        elsif i = cycles_per_sample-1 then
          i := 0;
        else
          i := i + 1;
          valid_out <= '0';
        end if;
      end if;
    end if;
  end process;

-- the incoming data stream may be changing when valid isn't asserted, but we
-- want to repeat the valid data in both channel slots. To do this, we'll
-- just latch the data every time the input valid signal is asserted.
latch_data : process(clk)
begin
  if rising_edge(clk) then
    if valid_in = '1'  then
      data_out <= data_in;
    end if;
  end if;
end process;

end architecture;
