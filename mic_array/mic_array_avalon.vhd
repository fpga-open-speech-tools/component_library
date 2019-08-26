-------------------------------------------------------------------------------------
--
--! @file       mic_array_avalon.vhd
--! @brief      Microphone array Avalon wrapper
--! @details    Avalon streaming and memory-mapped interface wrapper for the
--!             microphone array subsystem; outputs data/channel/valid for
--!             the mic array TDM serial data, and allows register control of
--!             measured propagation delay.
--! @author     Trevor Vannoy
--! @date       January 2019
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
library work;                 --! Use local library.
use ieee.std_logic_1164.all;  --! Use standard logic elements.
use ieee.numeric_std.all;     --! Use numeric standard.
use work.mic_array_param.all; --! Use mic array related constant parameters

----------------------------------------------------------------------------
--
--! @brief    mic_array_avalon
--! @details  Avalon streaming and memory-mapped interface wrapper for the
--!           microphone array subsystem; outputs data/channel/valid for
--!           the mic array TDM serial data, and allows register control of
--!           measured propagation delay.
-- TODO: update documentation!!!!
--! @param  sys_clk           Fabric system clock. Syncrhonous to sck, but faster.
--! @param  sck_master        Master SCK clock domain for all mic arrays
--! @param  sck_rcv           Recieve clock for incoming mic array data
--! @param  rst               Asynchronous active-high reset
--! @param  avs_s1_address    Avalon bus address
--! @param  avs_s1_write      Avalon bus write enable flag
--! @param  avs_s1_writedata  Avalon bus write data
--! @param  avs_s1_read       Avalon bus read enable flag
--! @param  avs_s1_readdata   Avalon bus read data
--! @param  ast_data          Streaming microphone array data
--! @param  ast_channel       Microphone array data stream TDM channel
--! @param  ast_valid         Avalon streaming valid flag, asserted at
--!                           the beginning of each TDM slot / word
--! @param  led_sd            RJ45 LED indicator signal for SD
--! @param  led_ws            RJ45 LED indicator signal for WS
--! @param  sd                Mic array SD data signal
--! @param  ws                Mic array WS sampling pulse
--
----------------------------------------------------------------------------
entity mic_array_avalon is
  port (
    ------------------------------------------------------------
    -- Clock and Reset Signals
    ------------------------------------------------------------
    sys_clk           : in  std_logic;
    sck_master        : in  std_logic;
    sck_rcv           : in  std_logic;
    rst               : in  std_logic;
    ------------------------------------------------------------
    -- Avalon Memory Mapped Slave Signals
    ------------------------------------------------------------
    avs_s1_address    : in  std_logic_vector(1 downto 0);
    avs_s1_write      : in  std_logic;
    avs_s1_writedata  : in  std_logic_vector(31 downto 0);
    avs_s1_read       : in  std_logic;
    avs_s1_readdata   : out std_logic_vector(31 downto 0);
    ------------------------------------------------------------
    -- Avalon Streaming Source Signals
    ------------------------------------------------------------
    ast_data          : out std_logic_vector(data_width - 1 downto 0);
    ast_channel       : out std_logic_vector(ch_width - 1 downto 0);
    ast_valid         : out std_logic;
    ------------------------------------------------------------
    -- External Conduit Signals
    ------------------------------------------------------------
    led_sd            : out std_logic;
    led_ws            : out std_logic;
    sd                : in  std_logic;
    ws                : out std_logic
  );
end entity;


architecture mic_array_avalon_arch of mic_array_avalon is

  ------------------------------------------------------------------------------------------
  -- Constant and Type Declarations
  ------------------------------------------------------------------------------------------

  ------------------------------------------------------------------------------------------
  -- Signal Declarations
  ------------------------------------------------------------------------------------------
  signal frame_delay_reg  : std_logic_vector(avs_s1_readdata'high downto 0) := (0 => '1', others => '0');
  signal data_old         : std_logic_vector(data_width-1 downto 0) := (others => '0');
  signal data             : std_logic_vector(data_width-1 downto 0) := (others => '0');

  ------------------------------------------------------------------------------------------
  -- Component Declerations
  ------------------------------------------------------------------------------------------
  component mic_array is
    port (
      sys_clk     : in  std_logic;
      sck_master  : in  std_logic;
      sck_rcv     : in  std_logic;
      rst         : in  std_logic;
      led_sd      : out std_logic;
      led_ws      : out std_logic;
      sd          : in  std_logic;
      ws          : out std_logic;
      frame_delay : in  integer;
      data        : out std_logic_vector(data_width - 1 downto 0);
      channel     : out std_logic_vector(ch_width - 1 downto 0);
      valid       : out std_logic
    );
  end component;

begin

  mic_array0 : component mic_array
    port map (
      sys_clk     => sys_clk,
      sck_master  => sck_master,
      sck_rcv     => sck_rcv,
      rst         => rst,
      led_sd      => led_sd,
      led_ws      => led_ws,
      sd          => sd,
      ws          => ws,
      frame_delay => to_integer(unsigned(frame_delay_reg)),
      data        => data,
      channel     => ast_channel
      -- valid       => ast_valid
    );

  -- TODO: add a "start" register so we can control when to start clocking the mic array

  -- read/write to registers
  process(sys_clk)
  begin
    if rising_edge(sys_clk) then
      -- read the registers
      if avs_s1_read = '1' then
        case avs_s1_address is
          when "00"   => avs_s1_readdata <=  frame_delay_reg;
          when others => avs_s1_readdata <= (others => '0');
        end case;
      -- write the registers
      elsif avs_s1_write = '1' then
        case avs_s1_address is
          when "00"   => frame_delay_reg <= avs_s1_writedata;
          when others => null;
        end case;
      end if;
    end if;
  end process;

  delay_data : process(sys_clk)
  begin
    if rising_edge(sys_clk) then
      data_old <= data;
    end if;
  end process;

  ast_valid <= '1' when data_old /= data else '0';
  ast_data <= data;

end architecture;
