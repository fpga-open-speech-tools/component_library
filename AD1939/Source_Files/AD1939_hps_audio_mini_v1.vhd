----------------------------------------------------------------------------------
--
-- Company:          Flat Earth, Inc.
-- Author/Engineer:  Ross K. Snider
--
-- Create Date:      06/29/2018
--
-- Design Name:      AD1939_hps_audio_mini.vhd  
--                      
-- Description:      The AD1939_hps_audio_mini component does the following:
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

entity AD1939_hps_audio_mini is
    port
    (   
        sys_clk                     : in     std_logic;  -- FPGA system fabric clock  (Note: sys_clk is assumed to be faster and synchronous to the AD1939 sample rate clock and bit clock, typically one generates sys_clk using a PLL that is N * AD1939_ADC_ALRCLK)
        sys_reset                   : in     std_logic;  -- FPGA system fabric reset
        -------------------------------------------------------------------------------------------------------------------------------------
        -- AD1939 Physical Layer : Signals to/from AD1939 Serial Data Port (from ADCs/to DACs), i.e. connection to physical pins on AD1939
        -- Note: ASDATA1 from ADC and DSDATA2, DSDATA3, DSDATA4 to DAC are not used since these are not present on the Audio Mini board.
        --       AD1939 register control is connected via the AD1939 SPI Control Port and controlled with the HPS SPI, so no control port in this component.
        -------------------------------------------------------------------------------------------------------------------------------------
        -- physical signals from ADC
        AD1939_ADC_ASDATA2          : in     std_logic;   -- Serial data from AD1939 pin 26 ASDATA2, ADC2 24-bit normal stereo serial mode
        AD1939_ADC_ABCLK            : in     std_logic;   -- Bit Clock from ADC (Master Mode) from pin 28 ABCLK on AD1939;  Note: bit clock = 64 * Fs, Fs = sample rate
        AD1939_ADC_ALRCLK           : in     std_logic;   -- Left/Right framing Clock from ADC (Master Mode) from pin 29 ALRCLK on AD1939;  Note: LR clock = Fs, Fs = sample rate
        -- physical signals to DAC
        AD1939_DAC_DSDATA1          : out    std_logic;   -- Serial data to AD1939 pin 20 DSDATA1, DAC1 24-bit normal stereo serial mode
        AD1939_DAC_DBCLK            : out    std_logic;   -- Bit Clock for DAC (Slave Mode) to pin 21 DBCLK on AD1939
        AD1939_DAC_DLRCLK           : out    std_logic;   -- Left/Right framing Clock for DAC (Slave Mode) to pin 22 DLRCLK on AD1939
        -----------------------------------------------------------------------------------------------------------
        -- Abstracted data channels, i.e. interface to the data plane as 24-bit data words.
        -- This is setup as a two channel Avalon Streaming Interface 
        -- See table 17, page 41, of Intel's Avalon® Streaming Interface Specifications
        -- https://www.altera.com/content/dam/altera-www/global/en_US/pdfs/literature/manual/mnl_avalon_spec.pdf
        -- Data is being clocked out at the sys_clk rate and valid is asserted only when data is present.  Left and right channels are specified as channel number (0 or 1)
        -- Data is converted to a W=32 (word length in bits), F=28 (number of fractional bits) before being sent out.
        -----------------------------------------------------------------------------------------------------------
        -- Avalon streaming interface from ADC to fabric
        AD1939_ADC_data         : out    std_logic_vector(31 downto 0);  -- W=32; F=28; Signed 2's Complement
        AD1939_ADC_channel      : out    std_logic_vector(1 downto 0);   -- Left <-> channel 0;  Right <-> channel 1
        AD1939_ADC_valid        : out    std_logic;                      -- asserted when data is valid 
        AD1939_ADC_error        : out    std_logic_vector(1 downto 0);   -- error channel is hard coded to zero since it is assumed no errors coming for ADC
        -- Avalon streaming interface to DAC from fabric
        AD1939_DAC_data         : in     std_logic_vector(31 downto 0);  -- W=32; F=28; Signed 2's Complement
        AD1939_DAC_channel      : in     std_logic_vector(1 downto 0);   -- Left <-> channel 0;  Right <-> channel 1
        AD1939_DAC_valid        : in     std_logic;                      -- asserted when data is valid 
        AD1939_DAC_error        : in     std_logic_vector(1 downto 0)   -- error channel is ignored, assumed to be error free at this point
    );
end AD1939_hps_audio_mini;

architecture behavioral of AD1939_hps_audio_mini is

    --------------------------------------------------------------
    -- Intel/Altera component to convert serial data to parallel
    --------------------------------------------------------------
    component Serial2Parallel_32bits
        PORT
        (
            clock           : IN STD_LOGIC ;
            shiftin         : IN STD_LOGIC ;
            q               : OUT STD_LOGIC_VECTOR (31 DOWNTO 0)
        );
    end component;

    --------------------------------------------------------------
    -- Intel/Altera component to convert parallel data to serial
    --------------------------------------------------------------
    component Parallel2Serial_32bits
        PORT
        (
            clock           : IN STD_LOGIC ;
            data            : IN STD_LOGIC_VECTOR (31 DOWNTO 0);
            load            : IN STD_LOGIC ;
            shiftout        : OUT STD_LOGIC 
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
    signal SregOut_ADC2             : std_logic_vector(31 downto 0);
    signal SregOut_ADC2_28          : std_logic_vector(27 downto 0);
    signal ADC2_data                : std_logic_vector(31 downto 0);
    signal DAC1_data_left           : std_logic_vector(23 downto 0);
    signal DAC1_data_right          : std_logic_vector(23 downto 0);
    signal AD1939_DAC_DSDATA1_left  : std_logic;
    signal AD1939_DAC_DSDATA1_right : std_logic;
    
    -- Register the output
    signal AD1939_ADC_valid_r       : std_logic;
    signal AD1939_ADC_channel_r     : std_logic_vector(1 downto 0);

begin

   ----------------------------------------------------------------------------
   -- The master bit clock from ADC drives the slave bit clock of DAC
   -- The master framing LR clock fro ADC drives the slave LR clock of DAC
   -- ADC is set as master for BCLK and LRCLK.  Set in ADC Control 2 Register.  
   -- See Table 25 on page 28 of AD1939 data sheet.
   ----------------------------------------------------------------------------
   AD1939_DAC_DBCLK  <= AD1939_ADC_ABCLK;   
   AD1939_DAC_DLRCLK <= AD1939_ADC_ALRCLK;
   AD1939_ADC_error  <= "00";  -- no errors coming from ADC so hardcode Avalon error signal as zero.

   
   -------------------------------------------------------------
   -- Convert serial data stream to parallel
   -------------------------------------------------------------
    S2P_ADC2 : Serial2Parallel_32bits PORT MAP (
        clock           => AD1939_ADC_ABCLK,   -- serial bit clock
        shiftin         => AD1939_ADC_ASDATA2, -- serial data in
        q               => SregOut_ADC2        -- parallel data out
    );
    
   -------------------------------------------------------------
   -- Get the 24-bits with a SDATA delay of 1 (SDATA delay set in ADC Control 1 Register; See Table 24 page 27 of AD1939 data sheet) 
   -- Then:  1. Add 4 more fractional bits to get a word that is W=28, F=28
   --        2. Sign extend to get a word that is W=32, F=28
   -------------------------------------------------------------    
    SregOut_ADC2_28  <= SregOut_ADC2(30 downto 7) & "0000";                                   -- grab 24 parallel bits and append 4 fractional bits
    ADC2_data        <= std_logic_vector(resize(signed(SregOut_ADC2_28), ADC2_data'length));  -- sign extend to W=32, the data format that is streamed to fabric

    --------------------------------------------------------------
    -- State Machine to implement Avalon streaming
    -- Logic to advance to the next state
    --------------------------------------------------------------
   process (sys_clk, sys_reset)
   begin
         if sys_reset = '1' then
               state <= state_left_wait;    
         elsif (rising_edge(sys_clk)) then
               case state is
                     ---------------------------------------------
                     -- left 
                     ---------------------------------------------                     
                     when state_left_wait =>
                           if AD1939_ADC_ALRCLK = '0' then       -- capture left data when ALRCK goes low
                                 state <= state_left_capture;   
                           else
                                 state <= state_left_wait;
                           end if;  
                     when state_left_capture =>                  -- state to capture data
                           state <= state_left_valid;  
                     when state_left_valid =>                    -- state to generate valid signal
                           state <= state_right_wait;
                     ---------------------------------------------
                     -- right
                     ---------------------------------------------                     
                     when state_right_wait =>
                           if AD1939_ADC_ALRCLK = '1' then       -- capture right data when ALRCK goes high
                                 state <= state_right_capture;
                           else
                                 state <= state_right_wait;
                           end if;  
                     when state_right_capture =>                 -- state to capture data
                           state <= state_right_valid;  
                     when state_right_valid =>
                           state <= state_left_wait;             -- state to generate valid signal                           
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
   process (sys_clk)
   begin
         if (rising_edge(sys_clk)) then
               AD1939_ADC_valid_r         <= '0';    -- default behavior is valid low  (signifies no data)
               AD1939_ADC_channel_r       <= "11";   -- default channel number is 3    (signifies no data)

               case state is
                     ---------------------------------------------
                     -- Get left sample
                     ---------------------------------------------                     
                     when state_left_wait =>   
                     when state_left_capture =>
                            AD1939_ADC_data          <= ADC2_data;                     -- send out data in W=32, F=28 format
                     when state_left_valid =>
                            AD1939_ADC_valid_r         <= '1';                           -- valid signal
                            AD1939_ADC_channel_r       <= "00";                          -- left channel is channel 0

                     ---------------------------------------------
                     -- Get right sample
                     ---------------------------------------------                     
                     when state_right_wait =>   
                     when state_right_capture =>
                            AD1939_ADC_data          <= ADC2_data;                     -- send out data in W=32, F=28 format
                     when state_right_valid =>
                            AD1939_ADC_valid_r         <= '1';                           -- valid signal
                            AD1939_ADC_channel_r       <= "01";                          -- right channel is channel 1

                     when others =>
                           -- do nothing
               end case;
         end if;
   end process;


   -------------------------------------------------------------
    -- Capture Avalon Streaming data that is to be sent to DAC1
    -- The streaming data is assumed to be normalized.
   -------------------------------------------------------------
   process (sys_clk)
   begin
         if (rising_edge(sys_clk)) then
             if AD1939_DAC_valid  = '1' then  -- data has arrived
                 case AD1939_DAC_channel is
                     when "00" => DAC1_data_left  <= AD1939_DAC_data(27 downto 4);  -- grab 24 bits out of the left channel that is W=32, F=28
                     when "01" => DAC1_data_right <= AD1939_DAC_data(27 downto 4);  -- grab 24 bits out of the right channel that is W=32, F=28
                     when others =>                                                 -- do nothing
                 end case;
             end if;
         end if;
   end process;

   -------------------------------------------------------------
    -- DAC1 Left
   -------------------------------------------------------------
    P2S_DAC1_left : Parallel2Serial_32bits PORT MAP (
        clock           => AD1939_ADC_ABCLK,
        data            => '0' & DAC1_data_left & "0000000",          -- Insert the 24-bits with a SDATA delay of 1 (SDATA delay set in DAC Control 0 Register; See Table 18 page 25 of AD1939 data sheet).
        load            => AD1939_ADC_ALRCLK,                     -- load: loads when high, component performs shift operation when low.  LRCLK -> Left Low so start shifting when LRCK goes low
        shiftout        => AD1939_DAC_DSDATA1_left
    );

   -------------------------------------------------------------
    -- DAC1 Right
   -------------------------------------------------------------
    P2S_DAC1_right : Parallel2Serial_32bits PORT MAP (
        clock           => AD1939_ADC_ABCLK,
        data            => '0' & DAC1_data_right & "0000000",         -- Insert the 24-bits with a SDATA delay of 1 (SDATA delay set in DAC Control 0 Register; See Table 18 page 25 of AD1939 data sheet).
        load            => not AD1939_ADC_ALRCLK,                         -- load: loads when high, component performs shift operation when low.  LRCLK -> Right High so start shifting when not LRCK goes low
        shiftout        => AD1939_DAC_DSDATA1_right
    );
    
   ------------------------------------------------------------------
   -- Interleave the left/right serial data that goes out to the DAC
   ------------------------------------------------------------------
    process (AD1939_ADC_ALRCLK)
    begin
        if (AD1939_ADC_ALRCLK = '0') then                                 
            AD1939_DAC_DSDATA1 <= AD1939_DAC_DSDATA1_left;  -- When LRCLK is 1, stream out the left channel   (Left-Justified Mode;  See Figure 23 on page 21 of AD1939 data sheet)
        else
            AD1939_DAC_DSDATA1 <= AD1939_DAC_DSDATA1_right;        
        end if;
    end process;
 
  -- Map the output ADC signals to the ports
  AD1939_ADC_valid    <= AD1939_ADC_valid_r;
  AD1939_ADC_channel  <= AD1939_ADC_channel_r;
  
end behavioral;
    
    
    
    