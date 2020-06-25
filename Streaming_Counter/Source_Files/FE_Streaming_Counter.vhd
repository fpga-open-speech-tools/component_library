----------------------------------------------------------------------------
--! @file FE_Streaming_Counter.vhd
--! @brief 
--! @details  Simple module that generates a counter that is passed over an 
--!           avalon streaming interface 
--! @author Tyler Davis
--! @date 2019
--! @copyright Copyright 2019 Audio Logic
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
-- Audio Logic
-- 985 Technology Blvd
-- Bozeman, MT 59718
-- support@flatearthinc.com
----------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

entity FE_Streaming_Counter is
    generic ( 
      n_channels : integer := 8;
      channel_width : integer := 8;
      data_width : integer := 32
      
    );
    port (
        sys_clk             : in  std_logic                     := '0';
        reset_n             : in  std_logic                     := '0';
        
        data_output_channel  : out  std_logic_vector(channel_width-1 downto 0)  := (others => '0');
        data_output_data     : out  std_logic_vector(data_width-1 downto 0) := (others => '0');
        data_output_error    : out  std_logic_vector(1 downto 0)  := (others => '0');
        data_output_valid    : out  std_logic                     := '0'
    );
    
end entity FE_Streaming_Counter;

architecture rtl of FE_Streaming_Counter is
 
signal delay_value    : unsigned(31 downto 0) := "00000010111011100000000000000000";  -- Time between data transfers

signal transmit_data  : std_logic := '0';

signal data_r         : std_logic_vector(data_width-1 downto 0) := (others => '0');
signal channel_r      : std_logic_vector(channel_width-1 downto 0) := (others => '0');
signal valid_r        : std_logic := '0';

-- Shifter state machine signals
signal channel_counter  : unsigned(channel_width-1 downto 0) := (others => '0');
signal delay_counter    : unsigned(31 downto 0) := (others => '0');

signal data_counter : std_logic_vector(data_width-1 downto 0) := (others => '0');

-- Create states for the streaming state machine
type state_type is (idle,increment_counter,pass_data); 
signal output_state : state_type;

begin 

-- Process to start the data transmission after X clock cycles
counter_process: process(sys_clk,reset_n)
begin
  if reset_n = '0' then 
    delay_counter   <= (others => '0');
  elsif rising_edge(sys_clk) then 
  
    -- If the counter reaches the defined value, pulse the transmit data signal and reset the counter
    if delay_counter = delay_value then 
      delay_counter <= (others => '0');
      transmit_data <= '1';
    else
      transmit_data <= '0';
      delay_counter <= delay_counter + 1;
    end if;
  end if;
end process;

-- Process to control the output data transfer state machine
control_process : process(sys_clk,reset_n)
begin
  if reset_n = '0' then 
  elsif rising_edge(sys_clk) then 
    case output_state is 
    
      when idle => 
      
        -- When the transmit signal is pulsed, move to the data passing state
        if transmit_data = '1' then 
          output_state <= pass_data;
          
        -- Otherwise stay idle
        else
          output_state <= idle;
        end if;
      
      when increment_counter =>
        output_state <= pass_data;
          
      
      when pass_data =>
      
        -- When the specified number of channels is reached, go idle
        if channel_counter = to_unsigned(n_channels - 1,channel_counter'length) then 
          output_state <= idle;
          
        -- Otherwise keep incrementing the counter and passing data
        else
          output_state <= increment_counter;
        end if;
             
      when others => 
    
    end case;
  end if;
end process;

data_process : process(sys_clk,reset_n)
begin
  if reset_n = '0' then 
    valid_r <= '0';
    channel_counter <= (others => '0');
  elsif rising_edge(sys_clk) then 
    case output_state is 
    
      when idle => 
      
        -- Reset the counter and deassert the valid signal
        channel_counter   <= (others => '0');
        valid_r           <= '0';
              
      when increment_counter =>
      
        -- Increment the counter data
        data_counter      <= std_logic_vector(unsigned(data_counter) + 1);
        
        -- Increment the channel number
        channel_counter   <= channel_counter + 1;
        
        -- Deassert the valid
        valid_r           <= '0';
      
      when pass_data =>
      
        -- Load the data into the output register
        data_r      <= "1111" & data_counter(data_width-9 downto 0) & "0000";
        
        -- Load the channel into the output register
        channel_r   <= std_logic_vector(channel_counter);
        
        -- Load the valid pulse into the output register
        valid_r     <= '1';
        
      when others => 
    
    end case;
  end if;
end process;

-- Map the registers to the ports
data_output_data      <= data_r;
data_output_channel   <= channel_r;
data_output_valid     <= valid_r;

end architecture rtl;























































