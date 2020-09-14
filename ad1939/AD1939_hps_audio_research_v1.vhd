----------------------------------------------------------------------------------
--
-- Company:          Audio Logic
-- Author/Engineer:  Ross K. Snider, Tyler B. Davis
--
-- Create Date:      06/29/2018
--
-- Design Name:      AD1939_hps_audio_research.vhd
--
-- Description:      The AD1939_hps_audio_research component does the following:
--                         1. Provides a simple Qsys data interface for to the AD1939 Audio Codec
--                         2. Note: It is assumed that the AD1939 SPI Control Interface is connected to the SPI master of the Cyclone V HPS, thus there is no control interface in this component.
--                            -- Signals to/from AD1939 SPI Control Port (data direction from AD1939 perspective), connection to physical pins on AD1939
--                            -- 10 MHz CCLK max (see page 7 of AD1939 data sheet)
--                            -- CIN data is 24-bits (see page 14 of AD1939 data sheet)
--                            -- CLATCH_n must have pull-up resistor so that AD1939 recognizes presence of SPI controller on power-up
--                         3. Note: To create a synchronous fabric system clock, run the AD1939 MCLK0 to a PLL in the FPGA to create appropriate multiples of the 12.288 audio clock.
--                         4. The Audio Mini card only has a stereo Line-In port and a stereo Headphone-Out port.  The other codec I/O channels are not used in the Audio Mini card and do not appear in the VHDL interface.
--                         5. AD1939 should be configured as follows:
--                               a.  Normal stereo serial mode (see page 15 of AD1939 data sheet)
--                               b.  24-bit word length
--                               c.  ADC set as Master
--                               d.  DAC set as Slave
--
-- Target Device(s): Terasic D1E0-Nano Board
-- Tool versions:    Quartus Prime 18.0
--
--
-- Revisions:        1.0 (File Created)
--
-- Additional Comments:
----------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

use work.ad1939.all;

entity ad1939_hps_audio_research is
  port (
    sys_clk   : in    std_logic; -- FPGA system fabric clock  (Note: sys_clk is assumed to be faster and synchronous to the AD1939 sample rate clock and bit clock, typically one generates sys_clk using a PLL that is N * AD1939_ADC_ALRCLK)
    sys_reset : in    std_logic; -- FPGA system fabric reset
    -------------------------------------------------------------------------------------------------------------------------------------
    -- AD1939 Physical Layer : Signals to/from AD1939 Serial Data Port (from ADCs/to DACs), i.e. connection to physical pins on AD1939
    -- Note: ASDATA1 from ADC and DSDATA2, DSDATA3, DSDATA4 to DAC are not used since these are not present on the Audio Mini board.
    --       AD1939 register control is connected via the AD1939 SPI Control Port and controlled with the HPS SPI, so no control port in this component.
    -------------------------------------------------------------------------------------------------------------------------------------
    -- physical signals from ADC
    ad1939_adc_asdata1 : in    std_logic; -- Serial data from AD1939 pin 26 ASDATA2, ADC2 24-bit normal stereo serial mode
    ad1939_adc_asdata2 : in    std_logic; -- Serial data from AD1939 pin 26 ASDATA2, ADC2 24-bit normal stereo serial mode
    ad1939_adc_abclk   : in    std_logic; -- Bit Clock from ADC (Master Mode) from pin 28 ABCLK on AD1939;  Note: bit clock = 64 * Fs, Fs = sample rate
    ad1939_adc_alrclk  : in    std_logic; -- Left/Right framing Clock from ADC (Master Mode) from pin 29 ALRCLK on AD1939;  Note: LR clock = Fs, Fs = sample rate

    -- physical signals to DAC
    ad1939_dac_dsdata1 : out   std_logic; -- Serial data to AD1939 pin 20 DSDATA1, DAC1 24-bit normal stereo serial mode
    ad1939_dac_dsdata2 : out   std_logic; -- Serial data to AD1939 pin 20 DSDATA1, DAC1 24-bit normal stereo serial mode
    ad1939_dac_dsdata3 : out   std_logic; -- Serial data to AD1939 pin 20 DSDATA1, DAC1 24-bit normal stereo serial mode
    ad1939_dac_dsdata4 : out   std_logic; -- Serial data to AD1939 pin 20 DSDATA1, DAC1 24-bit normal stereo serial mode

    ad1939_dac_dbclk  : out   std_logic; -- Bit Clock for DAC (Slave Mode) to pin 21 DBCLK on AD1939
    ad1939_dac_dlrclk : out   std_logic; -- Left/Right framing Clock for DAC (Slave Mode) to pin 22 DLRCLK on AD1939

    -----------------------------------------------------------------------------------------------------------
    -- Abstracted data channels, i.e. interface to the data plane as 24-bit data words.
    -- This is setup as a two channel Avalon Streaming Interface
    -- See table 17, page 41, of Intel's Avalon� Streaming Interface Specifications
    -- https://www.altera.com/content/dam/altera-www/global/en_US/pdfs/literature/manual/mnl_avalon_spec.pdf
    -- Data is being clocked out at the sys_clk rate and valid is asserted only when data is present.  Left and right channels are specified as channel number (0 or 1)
    -- Data is converted to a W=32 (word length in bits), F=28 (number of fractional bits) before being sent out.
    -----------------------------------------------------------------------------------------------------------
    -- Avalon streaming interface from ADC to fabric
    ad1939_adc1_data    : out   std_logic_vector(word_length - 1 downto 0); -- W=32; F=28; Signed 2's Complement
    ad1939_adc1_channel : out   std_logic;  -- Left <-> channel 0;  Right <-> channel 1
    ad1939_adc1_valid   : out   std_logic;                     -- asserted when data is valid

    ad1939_adc2_data    : out   std_logic_vector(word_length - 1 downto 0); -- W=32; F=28; Signed 2's Complement
    ad1939_adc2_channel : out   std_logic;  -- Left <-> channel 0;  Right <-> channel 1
    ad1939_adc2_valid   : out   std_logic;                     -- asserted when data is valid

    -- Avalon streaming interface to DAC from fabric
    ad1939_dac1_data    : in    std_logic_vector(word_length - 1 downto 0); -- W=32; F=28; Signed 2's Complement
    ad1939_dac1_channel : in    std_logic;  -- Left <-> channel 0;  Right <-> channel 1
    ad1939_dac1_valid   : in    std_logic;                     -- asserted when data is valid

    ad1939_dac2_data    : in    std_logic_vector(word_length - 1 downto 0); -- W=32; F=28; Signed 2's Complement
    ad1939_dac2_channel : in    std_logic;  -- Left <-> channel 0;  Right <-> channel 1
    ad1939_dac2_valid   : in    std_logic;                     -- asserted when data is valid

    ad1939_dac3_data    : in    std_logic_vector(word_length - 1 downto 0); -- W=32; F=28; Signed 2's Complement
    ad1939_dac3_channel : in    std_logic;  -- Left <-> channel 0;  Right <-> channel 1
    ad1939_dac3_valid   : in    std_logic;                     -- asserted when data is valid

    ad1939_dac4_data    : in    std_logic_vector(word_length - 1 downto 0); -- W=32; F=28; Signed 2's Complement
    ad1939_dac4_channel : in    std_logic;  -- Left <-> channel 0;  Right <-> channel 1
    ad1939_dac4_valid   : in    std_logic                     -- asserted when data is valid
  );
end entity ad1939_hps_audio_research;

architecture behavioral of ad1939_hps_audio_research is

  --------------------------------------------------------------
  -- Intel/Altera component to convert serial data to parallel
  --------------------------------------------------------------

  component serial2parallel_32bits is
    port (
      clock   : in    std_logic;
      shiftin : in    std_logic;
      q       : out   std_logic_vector(31 downto 0)
    );
  end component;

  --------------------------------------------------------------
  -- Intel/Altera component to convert parallel data to serial
  --------------------------------------------------------------

  component parallel2serial_32bits is
    port (
      clock    : in    std_logic;
      data     : in    std_logic_vector(31 downto 0);
      load     : in    std_logic;
      shiftout : out   std_logic
    );
  end component;

  ---------------------------------------------------------------------------
  -- State Machine states to implement Avalon streaming with valid signal
  -- See I2S-Justified Mode in Figure 23 on page 21 of AD1939 data sheet.
  ---------------------------------------------------------------------------

  type state_type is (state_left_wait, state_left_capture, state_left_valid, state_right_wait, state_right_capture, state_right_valid);

  signal state : state_type;

  --------------------------------------------------------------
  -- Internal Signals
  --------------------------------------------------------------
  -- ADC signals
  signal sregout_adc1    : std_logic_vector(31 downto 0);
  signal sregout_adc1_24 : std_logic_vector(word_length - 1 downto 0);
  signal adc1_data       : std_logic_vector(word_length - 1 downto 0);

  signal sregout_adc2    : std_logic_vector(31 downto 0);
  signal sregout_adc2_24 : std_logic_vector(word_length - 1 downto 0);
  signal adc2_data       : std_logic_vector(word_length - 1 downto 0);

  -- DAC signals
  signal dac1_data_left           : std_logic_vector(word_length - 1 downto 0);
  signal dac1_data_right          : std_logic_vector(word_length - 1 downto 0);
  signal ad1939_dac_dsdata1_left  : std_logic;
  signal ad1939_dac_dsdata1_right : std_logic;

  signal dac2_data_left           : std_logic_vector(word_length - 1 downto 0);
  signal dac2_data_right          : std_logic_vector(word_length - 1 downto 0);
  signal ad1939_dac_dsdata2_left  : std_logic;
  signal ad1939_dac_dsdata2_right : std_logic;

  signal dac3_data_left           : std_logic_vector(word_length - 1 downto 0);
  signal dac3_data_right          : std_logic_vector(word_length - 1 downto 0);
  signal ad1939_dac_dsdata3_left  : std_logic;
  signal ad1939_dac_dsdata3_right : std_logic;

  signal dac4_data_left           : std_logic_vector(word_length - 1 downto 0);
  signal dac4_data_right          : std_logic_vector(word_length - 1 downto 0);
  signal ad1939_dac_dsdata4_left  : std_logic;
  signal ad1939_dac_dsdata4_right : std_logic;

  -- Register the output
  signal ad1939_adc1_valid_r   : std_logic;
  signal ad1939_adc1_channel_r : std_logic;
  signal ad1939_adc2_valid_r   : std_logic;
  signal ad1939_adc2_channel_r : std_logic;

begin

  ----------------------------------------------------------------------------
  -- The master bit clock from ADC drives the slave bit clock of DAC
  -- The master framing LR clock fro ADC drives the slave LR clock of DAC
  -- ADC is set as master for BCLK and LRCLK.  Set in ADC Control 2 Register.
  -- See Table 25 on page 28 of AD1939 data sheet.
  ----------------------------------------------------------------------------
  ad1939_dac_dbclk  <= ad1939_adc_abclk;
  ad1939_dac_dlrclk <= ad1939_adc_alrclk;

  -------------------------------------------------------------
  -- Convert serial data stream to parallel
  -------------------------------------------------------------
  s2p_adc1 : serial2parallel_32bits
    port map (
      clock   => ad1939_adc_abclk,
      shiftin => ad1939_adc_asdata1,
      q       => sregout_adc1
    );

  s2p_adc2 : serial2parallel_32bits
    port map (
      clock   => ad1939_adc_abclk,
      shiftin => ad1939_adc_asdata2,
      q       => sregout_adc2
    );

  -------------------------------------------------------------
  -- Get the 24-bits with a SDATA delay of 1 (SDATA delay set in ADC Control 1 Register; See Table 24 page 27 of AD1939 data sheet)
  sregout_adc1_24 <= sregout_adc1(30 downto 7); 
  sregout_adc2_24 <= sregout_adc2(30 downto 7); 

  --------------------------------------------------------------
  -- State Machine to implement Avalon streaming
  -- Logic to advance to the next state
  --------------------------------------------------------------
  process (sys_clk, sys_reset) is
  begin

    if (sys_reset = '1') then
      state <= state_left_wait;
    elsif (sys_clk'event and sys_clk = '1') then

      case state is

        ---------------------------------------------
        -- left
        ---------------------------------------------
        when state_left_wait =>
          if (ad1939_adc_alrclk = '0') then -- capture left data when ALRCK goes low
            state <= state_left_capture;
          else
            state <= state_left_wait;
          end if;
        when state_left_capture =>          -- state to capture data
          state <= state_left_valid;
        when state_left_valid =>            -- state to generate valid signal
          state <= state_right_wait;
        ---------------------------------------------
        -- right
        ---------------------------------------------
        when state_right_wait =>
          if (ad1939_adc_alrclk = '1') then -- capture right data when ALRCK goes high
            state <= state_right_capture;
          else
            state <= state_right_wait;
          end if;
        when state_right_capture =>         -- state to capture data
          state <= state_right_valid;
        when state_right_valid =>
          state <= state_left_wait;         -- state to generate valid signal
        ---------------------------------------------
        -- catch all
        ---------------------------------------------
        when others =>
          state <= state_left_wait;

      end case;

    end if;

  end process;

  --------------------------------------------------------------
  -- State Machine to implement Avalon streaming
  -- Generate Avalon streaming signals
  --------------------------------------------------------------
  process (sys_clk) is
  begin

    if (sys_clk'event and sys_clk = '1') then
      ad1939_adc1_valid_r   <= '0';  -- default behavior is valid low  (signifies no data)

      case state is

        ---------------------------------------------
        -- Get left sample
        ---------------------------------------------
        when state_left_wait =>
        when state_left_capture =>
          ad1939_adc1_data <= adc1_data; -- send out data in W=32, F=28 format
          ad1939_adc2_data <= adc2_data; -- send out data in W=32, F=28 format
        when state_left_valid =>
          ad1939_adc1_valid_r   <= '1';  -- valid signal
          ad1939_adc1_channel_r <= '0'; -- left channel is channel 0
          ad1939_adc2_valid_r   <= '1';  -- valid signal
          ad1939_adc2_channel_r <= '0'; -- left channel is channel 0
        ---------------------------------------------
        -- Get right sample
        ---------------------------------------------
        when state_right_wait =>
        when state_right_capture =>
          ad1939_adc1_data <= adc1_data; -- send out data in W=32, F=28 format
          ad1939_adc2_data <= adc2_data; -- send out data in W=32, F=28 format

        when state_right_valid =>
          ad1939_adc1_valid_r   <= '1';  -- valid signal
          ad1939_adc1_channel_r <= '1'; -- right channel is channel 1
          ad1939_adc2_valid_r   <= '1';  -- valid signal
          ad1939_adc2_channel_r <= '1'; -- right channel is channel 1

        when others =>
          -- do nothing

      end case;

    end if;

  end process;

  -------------------------------------------------------------
  -- Capture Avalon Streaming data that is to be sent to DAC1
  -- The streaming data is assumed to be normalized.
  -------------------------------------------------------------
  process (sys_clk) is
  begin

    if (sys_clk'event and sys_clk = '1') then
      if (ad1939_dac1_valid  = '1') then -- data has arrived

        case ad1939_dac1_channel is

          when '0' =>
            dac1_data_left <= ad1939_dac1_data; 
          when '1' =>
            dac1_data_right <= ad1939_dac1_data; 
          when others =>                                      -- do nothing

        end case;

      end if;
    end if;

  end process;

  process (sys_clk) is
  begin

    if (sys_clk'event and sys_clk = '1') then
      if (ad1939_dac2_valid  = '1') then -- data has arrived

        case ad1939_dac2_channel is

          when '0' =>
            dac2_data_left <= ad1939_dac2_data;
          when '1' =>
            dac2_data_right <= ad1939_dac2_data;
          when others =>                                      -- do nothing

        end case;

      end if;
    end if;

  end process;

  process (sys_clk) is
  begin

    if (sys_clk'event and sys_clk = '1') then
      if (ad1939_dac3_valid  = '1') then -- data has arrived

        case ad1939_dac3_channel is

          when '0' =>
            dac3_data_left <= ad1939_dac3_data;
          when '1' =>
            dac3_data_right <= ad1939_dac3_data;
          when others =>                                      -- do nothing

        end case;

      end if;
    end if;

  end process;

  process (sys_clk) is
  begin

    if (sys_clk'event and sys_clk = '1') then
      if (ad1939_dac4_valid  = '1') then -- data has arrived

        case ad1939_dac4_channel is

          when '0' =>
            dac4_data_left <= ad1939_dac4_data;
          when '1' =>
            dac4_data_right <= ad1939_dac4_data;
          when others =>                                      -- do nothing

        end case;

      end if;
    end if;

  end process;

  -------------------------------------------------------------
  -- dac1 left
  -------------------------------------------------------------
  p2s_dac1_left : parallel2serial_32bits
    port map (
      clock    => ad1939_adc_abclk,
      data     => '0' & dac1_data_left & "0000000",
      load     => ad1939_adc_alrclk,
      shiftout => ad1939_dac_dsdata1_left
    );

  -------------------------------------------------------------
  -- dac1 right
  -------------------------------------------------------------
  p2s_dac1_right : parallel2serial_32bits
    port map (
      clock    => ad1939_adc_abclk,
      data     => '0' & dac1_data_right & "0000000",
      load     => not ad1939_adc_alrclk,
      shiftout => ad1939_dac_dsdata1_right
    );

  -------------------------------------------------------------
  -- dac2 left
  -------------------------------------------------------------
  p2s_dac2_left : parallel2serial_32bits
    port map (
      clock    => ad1939_adc_abclk,
      data     => '0' & dac2_data_left & "0000000",
      load     => ad1939_adc_alrclk,
      shiftout => ad1939_dac_dsdata2_left
    );

  -------------------------------------------------------------
  -- dac2 right
  -------------------------------------------------------------
  p2s_dac2_right : parallel2serial_32bits
    port map (
      clock    => ad1939_adc_abclk,
      data     => '0' & dac2_data_right & "0000000",
      load     => not ad1939_adc_alrclk,
      shiftout => ad1939_dac_dsdata2_right
    );

  -------------------------------------------------------------
  -- dac3 left
  -------------------------------------------------------------
  p2s_dac3_left : parallel2serial_32bits
    port map (
      clock    => ad1939_adc_abclk,
      data     => '0' & dac3_data_left & "0000000",
      load     => ad1939_adc_alrclk,
      shiftout => ad1939_dac_dsdata3_left
    );

  -------------------------------------------------------------
  -- dac3 right
  -------------------------------------------------------------
  p2s_dac3_right : parallel2serial_32bits
    port map (
      clock    => ad1939_adc_abclk,
      data     => '0' & dac3_data_right & "0000000",
      load     => not ad1939_adc_alrclk,
      shiftout => ad1939_dac_dsdata3_right
    );

  -------------------------------------------------------------
  -- dac4 left
  -------------------------------------------------------------
  p2s_dac4_left : parallel2serial_32bits
    port map (
      clock    => ad1939_adc_abclk,
      data     => '0' & dac4_data_left & "0000000",
      load     => ad1939_adc_alrclk,
      shiftout => ad1939_dac_dsdata4_left
    );

  -------------------------------------------------------------
  -- dac4 right
  -------------------------------------------------------------
  p2s_dac4_right : parallel2serial_32bits
    port map (
      clock    => ad1939_adc_abclk,
      data     => '0' & dac4_data_right & "0000000",
      load     => not ad1939_adc_alrclk,
      shiftout => ad1939_dac_dsdata4_right
    );

  ------------------------------------------------------------------
  -- Interleave the left/right serial data that goes out to the DAC
  ------------------------------------------------------------------
  process (AD1939_ADC_ALRCLK, ad1939_dac_dsdata1_left,
    ad1939_dac_dsdata1_right, ad1939_dac_dsdata2_left,
    ad1939_dac_dsdata2_right, ad1939_dac_dsdata3_left,
    ad1939_dac_dsdata3_right, ad1939_dac_dsdata4_left,
    ad1939_dac_dsdata4_right) is
  begin

    if (AD1939_ADC_ALRCLK = '0') then
      ad1939_dac_dsdata1 <= ad1939_dac_dsdata1_left;
      ad1939_dac_dsdata2 <= ad1939_dac_dsdata2_left;
      ad1939_dac_dsdata3 <= ad1939_dac_dsdata3_left;
      ad1939_dac_dsdata4 <= ad1939_dac_dsdata4_left;
    else
      ad1939_dac_dsdata1 <= ad1939_dac_dsdata1_right;
      ad1939_dac_dsdata2 <= ad1939_dac_dsdata2_right;
      ad1939_dac_dsdata3 <= ad1939_dac_dsdata3_right;
      ad1939_dac_dsdata4 <= ad1939_dac_dsdata4_right;
    end if;

  end process;

  -- Map the output ADC signals to the ports
  ad1939_adc1_valid   <= ad1939_adc1_valid_r;
  ad1939_adc1_channel <= ad1939_adc1_channel_r;

  ad1939_adc2_valid   <= ad1939_adc2_valid_r;
  ad1939_adc2_channel <= ad1939_adc2_channel_r;

end architecture behavioral;