------------------------------------------------------------------------------------------
--
--! @file       mic_array.vhd
--! @brief      Microphone array subsystem
--! @details    Wrapper for all of the components needed to successfully capture data
--!             from a microphone array and get it ready for further processing.
--! @author     Trevor Vannoy
--! @date       August 2018
--! @copyright  Copyright (C) 2018-2019 Trevor Vannoy
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
-- TODO: update documentation!
--
--! @brief    mic_array
--! @details  Wrapper for all of the components needed successfully capture data
--!           from a microphone array and get it ready for further processing.
--! @param    sys_clk       Fabric system clock. This is synchronous to sck, but faster.
--! @param    sck_master    Master SCK clock domain for all mic arrays
--! @param    sck_rcv       Recieve clock for incoming mic array data
--! @param    rst           Asynchronous active-high reset
--! @param    led_sd        RJ45 LED indicator signal for SD
--! @param    led_ws        RJ45 LED indicator signal for WS
--! @param    sd            Mic array SD data signal
--! @param    ws            Mic array WS sampling pulse
--! @param    frame_delay   TDM data frame propagation delay in SCK cycles
--
----------------------------------------------------------------------------
entity mic_array is
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
end entity;


architecture mic_array_arch of mic_array is

  ------------------------------------------------------------------------------------------
  -- Constant and Type Declarations
  ------------------------------------------------------------------------------------------

  ------------------------------------------------------------------------------------------
  -- Signal Declarations
  ------------------------------------------------------------------------------------------
  signal sd_valid         : boolean := false;
  signal frame_start      : std_logic;
  signal frame_start_tmp  : std_logic;
  signal data_tmp         : std_logic_vector(data_width - 1 downto 0);
  signal channel_tmp      : std_logic_vector(ch_width - 1 downto 0);
  signal valid_tmp        : std_logic;
  signal word_clk         : std_logic;
  signal word_clk_tmp     : std_logic;
  signal sd_delayed       : std_logic;
  signal ws_tmp           : std_logic;

  ------------------------------------------------------------------------------------------
  -- Component Declerations
  ------------------------------------------------------------------------------------------
  -- performs startup sequence for mic array and generates WS signal
  component mic_array_startup is
  port (
    sck         : in  std_logic;
    rst         : in  std_logic;
    data_valid  : out boolean := false;
    ws           : out std_logic
  );
  end component;

  -- send out a pulse at the beginning of every received TDM frame
  component frame_start_counter is
  port (
    sck         : in  std_logic;
    rst         : in  std_logic;
    frame_delay : in  integer;
    frame_start : out std_logic
    );
  end component;

  -- deserialize incoming mic array data
  component mic_array_deserializer is
  port (
    clk         : in  std_logic;
    rst         : in  std_logic;
    frame_start : in  std_logic;
    din         : in  std_logic;
    dout        : out std_logic_vector(data_width-1 downto 0) := (others => '0');
    word_clock  : out std_logic;
    channel     : out std_logic_vector(ch_width - 1 downto 0);
    valid       : out std_logic
    );
  end component;

  -- generic data synchronizer
  component synchronizer is
    generic (
      data_width : integer := 1
    );
    port (
      clk   : in  std_logic;
      rst   : in  std_logic;
      din   : in  std_logic_vector(data_width - 1 downto 0);
      dout  : out std_logic_vector(data_width - 1 downto 0)
    );
  end component;

  -- delay incoming data bits so all arrays are synchronous to each other
  component tapped_delay_line is
    generic (
      length : integer := 16
    );
    port (
      clk   : in  std_logic;
      rst   : in  std_logic;
      din   : in  std_logic;
      sel   : in  integer range 0 to length - 1;
      dout  : out std_logic := '0'
    );
  end component;

begin

  -- drive the RJ45 LEDs so we know the hardware is doing something.
  led_sd  <= not sd; -- sd is normally high; inverting it makes sure the status LED is off when there isn't data
  led_ws  <= ws_tmp;

  -- drive the mic array
  ws      <= ws_tmp;

  mic_array_startup0 : mic_array_startup
    port map (
      sck        => sck_master,
      rst        => rst,
      data_valid => sd_valid,
      ws          => ws_tmp
    );

  -- with the way the tapped_delay_line is set up, the beginning of the data frame
  -- will always start at delay_line_length cycles after sending out the WS pulse.
  frame_start_counter0 : frame_start_counter
    port map (
      sck          => sck_rcv,
      rst          => rst,
      frame_delay  => delay_line_length,
      frame_start  => frame_start_tmp
    );

  deserializer : mic_array_deserializer
    port map (
      clk         => sck_rcv,
      rst         => rst,
      frame_start => frame_start_tmp,
      din         => sd_delayed,
      dout        => data_tmp,
      word_clock  => word_clk_tmp,
      channel     => channel_tmp,
      valid       => valid_tmp
    );

  -- the tap number is selected so the effective delay on the data line equals the length
  -- of the delay line; this way, each array's data frames will be synchronous with each other.
  tapped_delay_line0 : tapped_delay_line
    port map (
      clk   => sck_rcv,
      rst   => rst,
      din   => sd,
      sel   => delay_line_length  - 1 - frame_delay,
      dout  => sd_delayed
    );

  -- synchronize mic data with the fabric system clock domain
  synchronize_data : synchronizer
    generic map (
      data_width => data_width
    )
    port map (
      clk   => sys_clk,
      rst   => rst,
      din   => data_tmp,
      dout  => data
    );

  -- synchronize TDM channel number with the fabric system clock domain
  synchronize_channel : synchronizer
    generic map (
      data_width => ch_width
    )
    port map (
      clk   => sys_clk,
      rst   => rst,
      din   => channel_tmp,
      dout  => channel
    );


  -- synchronize data word clock with the fabric system clock domain
  synchronize_word_clk : synchronizer
    port map (
      clk     => sys_clk,
      rst     => rst,
      din(0)  => word_clk_tmp,
      dout(0) => word_clk
    );

end architecture;
