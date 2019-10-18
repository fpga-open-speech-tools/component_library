----------------------------------------------------------------------------
--! @file Speaker_Array_Decoder.vhd
--! @brief Speaker array decoder component
--! @details  
--! @author Tyler Davis
--! @date 2019
--! @copyright Copyright 2019 Flat Earth Inc
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
-- Tyler Davis
-- Flat Earth Inc
-- 985 Technology Blvd
-- Bozeman, MT 59718
-- support@flatearthinc.com
----------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

entity Speaker_Array_Decoder is
    port (
        sys_clk             : in  std_logic                     := '0';
        mclk                : in  std_logic                     := '0';
        reset_n             : in  std_logic                     := '0';
                
        serial_data         : in  std_logic;
        serial_control      : in  std_logic;
        serial_clk          : in  std_logic;
        
        bclk_out            : out std_logic;
        lrclk_out           : out std_logic;
        sdata_out           : out std_logic
    );
    
end entity Speaker_Array_Decoder;

architecture rtl of Speaker_Array_Decoder is
--------------------------------------------------------------
-- Intel/Altera component to convert serial data to parallel
--------------------------------------------------------------
component Serial2Parallel_32bits
  PORT
  (
    clock           : in std_logic ;
    shiftin         : in std_logic ;
    q               : OUT std_logic_vector (31 DOWNTO 0)
  );
end component;


component FE_I2S_M10K
  port (
    mclk_in             : in std_logic                      := '0';
    sys_clk             : in  std_logic                     := '0';
    reset_n             : in  std_logic                     := '0';
    
    data_input_channel  : in  std_logic_vector(0 downto 0)  := (others => '0');
    data_input_data     : in  std_logic_vector(31 downto 0) := (others => '0');
    data_input_error    : in  std_logic_vector(1 downto 0)  := (others => '0');
    data_input_valid    : in  std_logic                     := '0';
            
    bclk_out            : out std_logic;
    lrclk_out           : out std_logic;
    sdata_out           : out std_logic
  );  
end component;

-- TODO tie this signal with the M-Map interface component
signal n_speakers     : unsigned(6 downto 0) := "0000010";

-- Control signals
signal start_shifting : std_logic := '0';
signal end_shifting   : std_logic := '0';
signal shift_busy     : std_logic := '0';

-- Shifter signals
signal shift_out        : std_logic;
signal shift_en_n       : std_logic := '0';

-- Shifter state machine signals
signal header_recieved  : std_logic := '0';
signal final_packet     : std_logic := '0';
signal bit_counter      : unsigned(4 downto 0) := (others => '0');
signal packet_counter   : unsigned(6 downto 0) := (others => '0');

-- Create states for the output state machine
type state_type is (idle,shift_header,shift_wait,shift_data,read_data,increment_read_address,shift_finish); 
signal output_state : state_type;

begin 

bclk_out <= '0';
lrclk_out <= '0';
sdata_out <= '0';

end architecture rtl;























































