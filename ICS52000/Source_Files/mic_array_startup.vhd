-------------------------------------------------------------------------------------
--
--! @file       mic_arary_startup.vhd
--! @brief      Mic array startup sequence
--! @details    Generate WS for mic array after the startup delay specified
--!             in the ICS-52000 datasheet
--! @author     Trevor Vannoy, Tyler Davis
--! @date       March 2020
--! @copyright  Copyright 2020 Audio Logic
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
--  Tyler Davis
--  Audio Logic
--  985 Technology Blvd
--  Bozeman, MT 59718
--  openspeech@flatearthinc.com
--  
------------------------------------------------------------------------------------------

library ieee;                  --! Use standard library.
use ieee.std_logic_1164.all;   --! Use standard logic elements.
use ieee.numeric_std.all;      --! Use numeric standard.

----------------------------------------------------------------------------
--
--! @brief    mic_array_startup
--! @details  Generate WS for mic array after the startup delay specified
--!           in the ICS-52000 datasheet, and set data_valid once the mic
--!           array data is valid (also specified in the datasheet)
--! @param    sck         Mic array clock
--! @param    rst         Asynchronous active-high reset
--! @param    data_valid  Data valid flag
--! @param    ws          WS mic array sampling clock
--
----------------------------------------------------------------------------
-- TODO: better entity name?
entity mic_array_startup is
  generic 
  (
    clk_frequency                 : integer := 24576000;
    startup_time_ms               : integer := 100;
    sck_cycles_until_data_valid   : integer := 262144;
    cycles_per_frame              : integer := 64
  );
  port 
  (
    sck           : in  std_logic;
    rst           : in  std_logic;
    data_valid    : out std_logic;
    ws            : out std_logic
  );
end entity;

architecture mic_array_startup_arch of mic_array_startup is

  ------------------------------------------------------------------------------------------
  -- Signal Declarations
  ------------------------------------------------------------------------------------------
  signal ws_enable            : std_logic := '0';
  signal ws_counter           : integer := 0;
  signal startup_counter      : integer := 0;
  signal data_valid_counter   : integer := 0;

  -- wait startup_sck_cycles before enabling ws.
  constant startup_sck_cycles : integer := clk_frequency / 1000 * startup_time_ms;


begin
    
  ws_enable_counter : process(sck, rst)
  begin
    if rst = '1' then
      startup_counter <= 0;
      ws_enable <= '0';
    elsif rising_edge(sck) then
      if startup_counter < startup_sck_cycles - 1 then
        startup_counter <= startup_counter + 1;
      else
        ws_enable <= '1';
      end if;
    end if;
  end process;

  -- -- generate the ws pulses with a 50% duty cycle
  -- generate_ws : process(sck, rst)
  -- begin
    -- if rst = '1' then
      -- ws_counter <= 0;
      -- ws <= '1';
    -- elsif rising_edge(sck) and ws_enable then
      -- if ws_counter < cycles_per_frame/2 then
        -- ws <= '1';
        -- ws_counter <= ws_counter + 1;
      -- elsif ws_counter < cycles_per_frame - 1 then
        -- ws <= '0';
        -- ws_counter <= ws_counter + 1;
      -- else
        -- ws <= '0';
        -- ws_counter <= 0;
      -- end if;
    -- end if;
  -- end process;

  -- pulse the word select line based on the falling edge of the serial clock
  generate_ws : process(sck, rst)
  begin
    if rst = '1' then
      ws_counter <= 0;
      ws <= '0';
    elsif falling_edge(sck) and ws_enable = '1' then
      if ws_counter < cycles_per_frame - 1 then
        ws <= '0';
        ws_counter <= ws_counter + 1;
      else
        ws <= '1';
        ws_counter <= 0;
      end if;
    end if;
  end process;

  -- set the data valid flag after sck_cycles_until_data_valid clock cycles
  set_data_valid_flag : process(sck, rst)
  begin
    if rst = '1' then
      data_valid_counter <= 0;
      data_valid <= '0';
    elsif rising_edge(sck) and ws_enable = '1' then
      if data_valid_counter < sck_cycles_until_data_valid - 1 then
        data_valid_counter <= data_valid_counter + 1;
      else
        data_valid <= '1';
      end if;
    end if;
  end process;

end architecture;
