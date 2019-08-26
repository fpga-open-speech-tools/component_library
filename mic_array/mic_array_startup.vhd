-------------------------------------------------------------------------------------
--
--! @file       mic_arary_startup.vhd
--! @brief      Mic array startup sequence
--! @details    Generate WS for mic array after the startup delay specified
--!             in the ICS-52000 datasheet
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
  port (
    sck         : in  std_logic;
    rst         : in  std_logic;
    data_valid  : out boolean := false;
    ws          : out std_logic
  );
end entity;

architecture mic_array_startup_arch of mic_array_startup is

  ------------------------------------------------------------------------------------------
  -- Constant and Type Declarations
  ------------------------------------------------------------------------------------------
  --! clock frequency in Hz
  constant clk_freq                       : integer := 24576000;
  --! number of clock cycles before sending the frame sync (WS) pulse; wait 20 ms (1/50 s) to send WS
  constant startup_sck_cycles             : integer := clk_freq/50;
  --! number of clock cycles after startup before the output data is valid; specified in ICS-52000 datasheet
  constant sck_cycles_until_data_valid    : integer := 262144;
  --! number of clock cycles per sampling frame
  constant cycles_per_frame               : integer := 512;

  ------------------------------------------------------------------------------------------
  -- Signal Declarations
  ------------------------------------------------------------------------------------------
  signal ws_enable            : boolean := false;
  signal ws_counter           : integer := 0;
  signal startup_counter      : integer := 0;
  signal data_valid_counter   : integer := 0;


begin
  -- wait startup_sck_cycles before enabling ws.
  -- the ICS5200 datasheet specifices to wait at least 10 ms before sending out ws pulses,
  ws_enable_counter : process(sck, rst)
  begin
    if rst = '1' then
      startup_counter <= 0;
      ws_enable <= false;
    elsif rising_edge(sck) then
      if startup_counter < startup_sck_cycles - 1 then
        startup_counter <= startup_counter + 1;
      else
        ws_enable <= true;
      end if;
    end if;
  end process;

  -- generate the ws pulses with a 50% duty cycle
  generate_ws : process(sck, rst)
  begin
    if rst = '1' then
      ws_counter <= 0;
      ws <= '1';
    elsif rising_edge(sck) and ws_enable then
      if ws_counter < cycles_per_frame/2 then
        ws <= '1';
        ws_counter <= ws_counter + 1;
      elsif ws_counter < cycles_per_frame - 1 then
        ws <= '0';
        ws_counter <= ws_counter + 1;
      else
        ws <= '0';
        ws_counter <= 0;
      end if;
    end if;
  end process;

  -- set the data valid flag after sck_cycles_until_data_valid clock cycles
  set_data_valid_flag : process(sck, rst)
  begin
    if rst = '1' then
      data_valid_counter <= 0;
      data_valid <= false;
    elsif rising_edge(sck) and ws_enable then
      if data_valid_counter < sck_cycles_until_data_valid - 1 then
        data_valid_counter <= data_valid_counter + 1;
      else
        data_valid <= true;
      end if;
    end if;
  end process;

end architecture;
