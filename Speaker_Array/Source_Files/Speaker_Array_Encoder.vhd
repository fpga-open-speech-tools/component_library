----------------------------------------------------------------------------
--! @file Speaker_Array_Encoder.vhd
--! @brief Speaker array encoder component
--! @details  This component takes streaming parallel data and converts it into a 
--!           streaming serial interface that can be transmitted via an RJ45
--!           interface.
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

entity Speaker_Array_Encoder is
    port (
        sys_clk             : in  std_logic                     := '0';
        reset_n             : in  std_logic                     := '0';
        
        data_input_channel  : in  std_logic_vector(6 downto 0)  := (others => '0');
        data_input_data     : in  std_logic_vector(31 downto 0) := (others => '0');
        data_input_error    : in  std_logic_vector(1 downto 0)  := (others => '0');
        data_input_valid    : in  std_logic                     := '0';
        
        led_sd              : out std_logic                     := '0';
        led_ws              : out std_logic                     := '0';
        
        serial_data_out     : out std_logic;
        serial_control      : out std_logic;
        clk_out             : out std_logic
    );
    
end entity Speaker_Array_Encoder;

architecture rtl of Speaker_Array_Encoder is
    
--------------------------------------------------------------
-- Altera component to convert parallel data to serial
--------------------------------------------------------------
component Parallel2Serial_32bits
  port
  (
    clock           : in std_logic ;
    data            : in std_logic_vector (31 downto 0);
    load            : in std_logic ;
    shiftout        : out std_logic 
  );
end component;

--------------------------------------------------------------
-- Altera DPR
--------------------------------------------------------------
component Encoder_DPR is
	port
	(
		clock		: in std_logic  := '1';
		data		: in std_logic_vector (31 downto 0);
		rdaddress		: in std_logic_vector (5 downto 0);
		rden		: in std_logic  := '1';
		wraddress		: in std_logic_vector (5 downto 0);
		wren		: in std_logic  := '0';
		q		: out std_logic_vector (31 downto 0)
	);
end component;

-- TODO tie this signal with the M-Map interface component
signal n_speakers     : unsigned(6 downto 0) := "0000010";

-- Control signals
signal start_shifting : std_logic := '0';
signal end_shifting   : std_logic := '0';
signal shift_busy     : std_logic := '0';

-- DPR Signals
signal wren           : std_logic := '0';
signal rden           : std_logic := '0';
signal read_address   : std_logic_vector(5 downto 0) := (others => '0');
signal write_address  : std_logic_vector(5 downto 0) := (others => '0');
signal input_data_r   : std_logic_vector(31 downto 0) := (others => '0');
signal output_data_r  : std_logic_vector(31 downto 0) := (others => '0');
signal shift_data_in  : std_logic_vector(31 downto 0) := (others => '0'); 

-- Shifter signals
signal shift_out      : std_logic;
signal shift_en_n       : std_logic := '0';

-- Shifter state machine signals
signal header_sent    : std_logic := '0';
signal final_packet   : std_logic := '0';
signal bit_counter    : unsigned(4 downto 0) := (others => '0');
signal packet_counter : unsigned(6 downto 0) := (others => '0');

-- Create states for the output state machine
type state_type is (idle,shift_header,shift_wait,shift_data,read_data,increment_read_address,shift_finish); 
signal output_state : state_type;

begin 

-- Map the DPR
encoder_buffer : Encoder_DPR
port map (
  clock => sys_clk,
  data => input_data_r,
  rdaddress => read_address,
  rden => rden,
  wraddress => write_address,
  wren => wren,
  q => output_data_r
);

-- Map the serializer
serial_data : Parallel2Serial_32bits
port map (
  clock => sys_clk,
  data => shift_data_in,
  load => shift_en_n,
  shiftout => shift_out
);

-- Process to push the data into the FIFO
data_in_process : process(sys_clk,reset_n)
begin 
  if reset_n = '0' then 
    write_address <= (others => '0');
    wren <= '0';
    input_data_r <= (others => '0');
  elsif rising_edge(sys_clk) then 
    -- Accept new data only when the valid is asserted
    if data_input_valid = '1' then 
      input_data_r <= data_input_data;
      
      -- Trim the address to fit the expected format
      -- Note: The input channel is larger than the FIFO address space
      -- to accomidate the AD1939 convention of setting the channel to 
      -- N + 1 when data is not being passed.  This may be changed in 
      -- later versions.
      write_address <= data_input_channel(5 downto 0);
      wren <= '1';
    -- Otherwise, reset the write enable and keep the current data
    else
      input_data_r <= input_data_r;
      write_address <= write_address;
      wren <= '0';
    end if;
  end if;
end process;

-- Process to start the bit shifting 
shift_start_process : process(sys_clk,reset_n)
begin 
  if reset_n = '0' then 
    start_shifting <= '0';
    shift_busy <= '0';
  elsif rising_edge(sys_clk) then 
  
    -- When the first data packet is recieved, start shifting the header out
    if write_address(5 downto 0) = "000000" then -- Note: SignalTap seems to mess with the comparison...
      start_shifting <= '1';
    else
      start_shifting <= '0';
    end if;
    
    -- When the shifting has started, assert the shift busy signal
    if start_shifting = '1' then 
      shift_busy <= '1';
      
    -- When the final bit has been shifted, assert the end shifting signal
    elsif end_shifting = '1' then 
      shift_busy <= '0';
    else
      shift_busy <= shift_busy;
    end if;  
  end if;
end process;

-- Process to control the states of the data shifting process
data_out_control_process : process(sys_clk,reset_n)
begin
  if reset_n = '0' then 
    output_state <= idle;
  elsif rising_edge(sys_clk) then 
  
    case output_state is 
    
      -- Idle (default) state
      when idle =>
      
        -- When the shift busy is asserted and the end signal is not asserted
        -- start shifting out the header
        if shift_busy = '1' and end_shifting = '0' then 
          output_state <= shift_header;
          
        -- Otherwise, stay idle
        else
          output_state <= idle;
        end if;
      
      when shift_header =>
        output_state <= shift_wait;
      
      when shift_data =>
        output_state <= increment_read_address;
      
      when increment_read_address =>
        output_state <= shift_wait;
      
      when shift_wait =>
      
        -- If the second to last bit has been shifted, change states
        if bit_counter = "11101" then 
        
          -- If the number of packets is equal to the number of speakers
          -- move the finish state
          if packet_counter = n_speakers then 
            output_state <= shift_finish;
            
          -- Otherwise, read the next data byte
          else 
            output_state <= read_data;
          end if;
        end if;
      
      when read_data =>
        output_state <= shift_data;
      
      when shift_finish =>
        output_state <= idle;
      
      when others => 
        output_state <= idle;
        
    end case;
    
  end if;
end process;

data_out_process : process(sys_clk,reset_n)
begin
  if reset_n = '0' then 
    end_shifting  <= '0';
  elsif rising_edge(sys_clk) then 
    case output_state is 
      when idle => 
      
        -- Reset the end_shifting signal and the bit counter
        end_shifting  <= '0';
        bit_counter   <= (others => '0');
      
      when shift_header =>
        -- Load the data header into the shift register and reset the read address
        --                |------||------||------| --> 25 bits remaining
        shift_data_in <= "0000000000000000000000000" & std_logic_vector(n_speakers); --TODO: Add more to header packet?
        shift_en_n <= '0';
        read_address <= (others => '0');
      
      when shift_data =>
      
        -- Load the audio data into the shift register
        shift_data_in <= output_data_r;
        
        -- Reset the bit counter
        bit_counter <= (others => '0');
        
        -- Disable the read enable and disable the shift register
        rden <= '0';
        shift_en_n <= '1';
        
      when increment_read_address =>
        -- Disable the read enable 
        rden <= '0';
        
        -- Increment the read address
        read_address <= std_logic_vector(unsigned(read_address) + 1);
        
        -- Start the shifting process
        shift_en_n <= '0';
      
      when shift_wait =>
        
        -- Increment the bit counter
        bit_counter <= bit_counter + 1;
      
      when read_data =>
      
        -- Assert the read enable signal and increment the bit counter
        rden <= '1';
        packet_counter <= packet_counter + 1;
      
      when shift_finish =>
      
        -- Assert te end shifting signal
        end_shifting <= '1';
        
        -- Disable the shift enable 
        shift_en_n <= '1';
        
        -- Reset the packet counter
        packet_counter <= (others => '0');
      
      when others => 
    
    end case;
  end if;
end process;


-- Map the RJ45 signals to the output ports
serial_data_out <= shift_out;
clk_out <= sys_clk and not shift_en_n;
serial_control <= '0'; -- TODO: add control components

end architecture rtl;























































