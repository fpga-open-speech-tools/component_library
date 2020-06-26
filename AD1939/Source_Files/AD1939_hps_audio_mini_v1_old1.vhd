----------------------------------------------------------------------------------
--
-- Company:          Audio Logic
-- Author/Engineer:  Ross K. Snider
--
-- Create Date:      06/29/2018
--
-- Design Name:      AD1939_hps_audio_mini.vhd  
--                      
-- Description:      The AD1939_hps_audio_mini component does the following:
--                         1. Provides a simple Qsys data interface for to the AD1939 Audio Codec 
--                         2. Note: It is assumed that the AD1939 SPI Control Interface is connected to the SPI master of the Cyclone V HPS, thus there is no control interface in this component.
                                                         -- Signals to/from AD1939 SPI Control Port (data direction from AD1939 perspective), connection to physical pins on AD1939
                                                         -- 10 MHz CCLK max (see page 7 of AD1939 data sheet)
                                                         -- CIN data is 24-bits (see page 14 of AD1939 data sheet)
                                                         -- CLATCH_n must have pull-up resistor so that AD1939 recognizes presence of SPI controller on power-up
--                         3. Note: To create a synchronous fabric system clock, run the AD1939 MCLK0 to a PLL in the FPGA to create appropriate multiples of the 12.288 audio clock.
--                         4. The Audio Mini card only has a stereo Line-In port and a stereo Headphone-Out port.  The other codec I/O channels are not used in the Audio Mini card and do not appear in the VHDL interface.
--                         5. AD1939 should be configured as follows:
--                               a.  Normal stereo serial mode (see page 15 of AD1939 data sheet)
--                               b.  24-bit word length
--                               c.  ADC set as Master
--                               c.  DAC set as Slave
--
-- Target Device(s): Terasic D1E0-Nano Board
-- Tool versions:    Quartus Prime 18.0
--
--
-- Revisions:        1.0 (File Created)
--
-- Additional Comments: 
----------------------------------------------------------------------------------

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

entity AD1939_hps_audio_mini is
    port
    (   
        reset                       : in     std_logic;
        -------------------------------------------------------------------------------------------------------------------------------------
        -- Physical Layer : Signals to/from AD1939 Serial Data Port (from ADCs/to DACs), i.e. connection to physical pins on AD1939
        -- Note: ASDATA1 from ADC and DSDATA2, DSDATA3, DSDATA4 to DAC are not used since these are not present in the Audio Mini board.
        -------------------------------------------------------------------------------------------------------------------------------------
        -- signals from ADC
        AD1939_ADC_ASDATA2          : in     std_logic;   -- Serial data from AD1939 pin 26 ASDATA2, ADC2 24-bit normal stereo serial mode
        AD1939_ADC_ABCLK            : in     std_logic;   -- Bit Clock from ADC (Master Mode) from pin 28 ABCLK on AD1939;  Note: bit clock = 64 * Fs, Fs = sample rate
        AD1939_ADC_ALRCLK           : in     std_logic;   -- Left/Right framing Clock from ADC (Master Mode) from pin 29 ALRCLK on AD1939;  Note: LR clock = Fs, Fs = sample rate
        -- signals to DAC
        AD1939_DAC_DSDATA1          : out    std_logic;   -- Serial data to AD1939 pin 20 DSDATA1, DAC1 24-bit normal stereo serial mode
        AD1939_DAC_DBCLK            : out    std_logic;   -- Bit Clock for DAC (Slave Mode) to pin 21 DBCLK on AD1939
        AD1939_DAC_DLRCLK           : out    std_logic;   -- Left/Right framing Clock for DAC (Slave Mode) to pin 22 DLRCLK on AD1939
        -----------------------------------------------------------------------------------------------------------
        -- Abstracted data channels, i.e. interface to the data plane as 24-bit data words.
        -----------------------------------------------------------------------------------------------------------
        -- Data from ADC
        AD1939_Data_ADC2_Left   : out std_logic_vector (23 downto 0); 
        AD1939_Data_ADC2_Right  : out std_logic_vector (23 downto 0); 
        --  Data to DAC 
        AD1939_Data_DAC1_Left   : in std_logic_vector (23 downto 0);    -- On the rising edge of the DAC_LRCLK, these DAC signals will be captured 
        AD1939_Data_DAC1_Right  : in std_logic_vector (23 downto 0)
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
    
    --------------------------------------------------------------
    -- Internal Signals
    --------------------------------------------------------------
    signal SregOut_ADC2_right      : std_logic_vector(31 downto 0);
    signal SregOut_ADC2_left       : std_logic_vector(31 downto 0);
    signal AD1939_DAC_DSDATA1_right : std_logic;
    signal AD1939_DAC_DSDATA1_left  : std_logic;

begin

   ----------------------------------------------------------------------------
   -- The master bit clock from ADC drives the slave bit clock of DAC
   -- The master framing LR clock fro ADC drives the slave LR clock of DAC
   -- ADC is set as master for BCLK and LRCLK.  Set in ADC Control 2 Register.  
   -- See Table 25 on page 28 of AD1939 data sheet.
   ----------------------------------------------------------------------------
   AD1939_DAC_DBCLK  <= AD1939_ADC_ABCLK;   
   AD1939_DAC_DLRCLK <= AD1939_ADC_ALRCLK;

   --------------------------------------------------------------------------------------------------- 
   
   -------------------------------------------------------------
   -- ADC2 Left
   -------------------------------------------------------------
    S2P_ADC2_left : Serial2Parallel_32bits PORT MAP (
        clock           => AD1939_ADC_ABCLK,
        shiftin         => AD1939_ADC_ASDATA2,
        q               => SregOut_ADC2_left
    );

    process (AD1939_ADC_ALRCLK)
    begin
        if ( falling_edge(AD1939_ADC_ALRCLK) ) then                       -- LRCLK -> When LR goes , left channel is present, so capture on falling edge. (Left-Justified Mode;  See Figure 23 on page 21 of AD1939 data sheet).
            AD1939_Data_ADC2_Left <= SregOut_ADC2_left(30 downto 7);     -- Get the 24-bits with a SDATA delay of 1 (SDATA delay set in ADC Control 1 Register; See Table 24 page 27 of AD1939 data sheet).
        end if;
    end process;

   -------------------------------------------------------------
   -- ADC2 Right
   -------------------------------------------------------------
    S2P_ADC2_right : Serial2Parallel_32bits PORT MAP (
        clock           => AD1939_ADC_ABCLK,
        shiftin         => AD1939_ADC_ASDATA2,
        q               => SregOut_ADC2_right
    );

    process (AD1939_ADC_ALRCLK)
    begin
        if ( rising_edge(AD1939_ADC_ALRCLK) ) then                        -- LRCLK -> When LR goes high, right channel is present, so capture on rising edge. (Left-Justified Mode;  See Figure 23 on page 21 of AD1939 data sheet).
            AD1939_Data_ADC2_Right <= SregOut_ADC2_right(30 downto 7);   -- Get the 24-bits with a SDATA delay of 1 (SDATA delay set in ADC Control 1 Register; See Table 24 page 27 of AD1939 data sheet).
        end if;
    end process;


   --------------------------------------------------------------------------------------------------- 
    
   -------------------------------------------------------------
    -- DAC1 Left
   -------------------------------------------------------------
    P2S_DAC1_left : Parallel2Serial_32bits PORT MAP (
        clock           => AD1939_ADC_ABCLK,
        data            => '0' & AD1939_Data_DAC1_left & "0000000",     -- Insert the 24-bits with a SDATA delay of 1 (SDATA delay set in DAC Control 0 Register; See Table 18 page 25 of AD1939 data sheet).
        load            => AD1939_ADC_ALRCLK,                            -- load: loads when high, component performs shift operation when low.  LRCLK -> Left Low so start shifting when LRCK goes low
        shiftout        => AD1939_DAC_DSDATA1_left
    );

   -------------------------------------------------------------
    -- DAC1 Right
   -------------------------------------------------------------
    P2S_DAC1_right : Parallel2Serial_32bits PORT MAP (
        clock           => AD1939_ADC_ABCLK,
        data            => '0' & AD1939_Data_DAC1_right & "0000000",     -- Insert the 24-bits with a SDATA delay of 1 (SDATA delay set in DAC Control 0 Register; See Table 18 page 25 of AD1939 data sheet).
        load            => not AD1939_ADC_ALRCLK,                         -- load: loads when high, component performs shift operation when low.  LRCLK -> Right High so start shifting when not LRCK goes low
        shiftout        => AD1939_DAC_DSDATA1_right
    );
    
   ------------------------------------------------------------------
   -- Interleave the left/right serial data that goes out to the DAC
   ------------------------------------------------------------------
    process (AD1939_ADC_ABCLK, AD1939_ADC_ALRCLK, AD1939_DAC_DSDATA1_left, AD1939_DAC_DSDATA1_right)
    begin
        if (AD1939_ADC_ALRCLK = '1') then                                 
            AD1939_DAC_DSDATA1 <= AD1939_DAC_DSDATA1_left;  -- When LRCLK is 1, stream out the left channel   (Left-Justified Mode;  See Figure 23 on page 21 of AD1939 data sheet)
        else
            AD1939_DAC_DSDATA1 <= AD1939_DAC_DSDATA1_right;        
        end if;
    end process;
 
    
        
end behavioral;
    
    
    
    
    
    
    