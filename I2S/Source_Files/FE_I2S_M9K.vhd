----------------------------------------------------------------------------
--! @file FE_I2S_M9K.vhd
--! @brief Component that converts a single left/right audio streaming interface to I2S
--! @details  This component converts a streaming multi-channel interface into an I2S signal.
--!           The input is a standard avalon streaming interface with up to 64 channels and 
--!           the output is a word up to 32 bits where each bit contains the left/right data.
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

entity FE_I2S_M9K is
  generic (
    bclk_div    : unsigned(7 downto 0)  := "00000100";
    lrclk_div   : unsigned(7 downto 0)  := "00010000";
    n_drivers   : unsigned(7 downto 0)  := "00100000";
    max_drivers : integer  := 32;
    read_ahead  : integer  := 1
  );
  port (
    mclk_in             : in std_logic                      := '0';
    sys_clk             : in  std_logic                     := '0';
    reset_n             : in  std_logic                     := '0';
    
    data_input_channel  : in  std_logic_vector(6 downto 0)  := (others => '0');
    data_input_data     : in  std_logic_vector(31 downto 0) := (others => '0');
    data_input_error    : in  std_logic_vector(1 downto 0)  := (others => '0');
    data_input_valid    : in  std_logic                     := '0';
            
    bclk_out            : out std_logic;
    lrclk_out           : out std_logic;
    sdata_out           : out std_logic_vector(max_drivers - 1 downto 0)
  );
    
end entity FE_I2S_M9K;

architecture rtl of FE_I2S_M9K is

-- Instantiate the component that shifts the data
component Gen_Shift_Container
  port (
  clk         : in  std_logic;
  input_data  : in  std_logic_vector(31 downto 0);
  output_data : out std_logic_vector(31 downto 0);
  load        : in  std_logic
  );
end component;

-- Instantiate the DPR
component I2S_DPR_M9K is
	port
	(
		clock		: in std_logic ;
		data		: in std_logic_vector (31 downto 0);
		rdaddress		: in std_logic_vector (6 downto 0);
		rden		: in std_logic  := '1';
		wraddress		: in std_logic_vector (6 downto 0);
		wren		: in std_logic  := '0';
		q		: out std_logic_vector (31 downto 0)
	);
end component;

-- Register array size declarations
type data_array is array (max_drivers - 1 downto 0) of std_logic_vector(31 downto 0);

-- Create the data registers
signal data_array_r  : data_array := (others => (others => '0'));
signal sdata_array_r : data_array := (others => (others => '0'));

-- Create an array of data used to delay the serial output by one bit clock
signal sdata_follower_r : data_array := (others => (others => '0'));

-- Create counters for the bit and lr clocks
signal bclk_counter  : unsigned(8 downto 0) := (others => '0');
signal lrclk_counter : unsigned(7 downto 0) := (others => '0');

-- Create the bit and lr clock registers
signal bclk_r : std_logic := '0';
signal lrclk_r : std_logic := '0';

-- Create the read control signals
signal read_trigger : std_logic := '0';
signal reading_data : std_logic := '0';
signal read_all     : std_logic := '0';
signal next_wait    : std_logic := '0';

signal load_data : std_logic := '0';

-- Create counters for the read data and which driver should be 
signal read_counter   : unsigned(7 downto 0) := (others => '0');
signal driver_counter : unsigned(7 downto 0) := (others => '0');

-- DPR Signals
signal wren           : std_logic := '0';
signal rden           : std_logic := '0';
signal read_address   : std_logic_vector(6 downto 0) := (others => '0');
signal write_address  : std_logic_vector(6 downto 0) := (others => '0');
signal input_data_r   : std_logic_vector(31 downto 0) := (others => '0');
signal output_data_r  : std_logic_vector(31 downto 0) := (others => '0');
signal data_in  : std_logic_vector(31 downto 0) := (others => '0'); 

-- Create states for the output state machine
type state_type is (idle,read_data,increment_read_address,read_finish,reg_data); 
signal output_state : state_type;

begin 
-- Map the DPR
i2s_buffer : I2S_DPR_M9K
port map (
  clock => sys_clk,
  data => input_data_r,
  rdaddress => read_address,
  rden => rden,
  wraddress => write_address,
  wren => wren,
  q => output_data_r
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
      write_address <= data_input_channel;
      wren <= '1';
    -- Otherwise, reset the write enable and keep the current data
    else
      input_data_r <= input_data_r;
      write_address <= write_address;
      wren <= '0';
    end if;
  end if;
end process;

bclk_process : process (mclk_in,reset_n)
begin 
  if reset_n = '0' then 
    bclk_r <= '0';
  elsif rising_edge(mclk_in) then 
  
    -- If the bitclock counter reached the divisor, reset it and change the bit clock signal
    if bclk_counter = bclk_div - 1 then 
      bclk_counter  <= (others => '0');
      bclk_r        <= not bclk_r;
      
    -- Otherwise increment the counter
    else
      bclk_counter  <= bclk_counter + 1;
    end if;
  end if;
end process;

lrclk_process : process(bclk_r,reset_n)
begin 
  if reset_n = '0' then 
    lrclk_r <= '0';
  elsif falling_edge(bclk_r) then 
  
    -- If the lr clock counter reached the divisor, reset it and change the bit clock signal
    if lrclk_counter = lrclk_div - 1 then 
      lrclk_r <= not lrclk_r;
      lrclk_counter <= (others => '0');
      
    -- Otherwise increment the counter
    else
      lrclk_counter <= lrclk_counter + 1;
    end if;
  end if;
end process;

read_trigger_process: process (sys_clk,reset_n)
begin 
  if reset_n = '0' then 

  elsif rising_edge(sys_clk) then 
  
    -- When the trigger signal is high and the data hasn't already been read
    -- start reading the data from the FIFO
    if read_trigger = '1' and next_wait = '0' then 
      reading_data <= '1';
      
    -- When the data has been read, stop the reading process
    elsif read_all = '1' then 
      reading_data <= '0';
      
    -- Otherwise keep the signal value
    else
      reading_data <= reading_data;
    end if;
  end if;
end process;

read_wait_process : process(sys_clk,reset_n)
begin 
  if reset_n = '0' then 
    next_wait <= '0';
    
  elsif rising_edge(sys_clk) then 
  
    -- Map the "wait" signal to the trigger (across a clock domain) to
    -- make sure the data isn't read multiple times
    if read_trigger = '1' then 
      next_wait <= '1';
    else
      next_wait <= '0';
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
    
      -- When idle, wait for the reading data signal to go high and make sure
      -- the data hasn't just been read out
      when idle =>
        if reading_data = '1' and read_all = '0' then 
          output_state <= read_data;
        else
          output_state <= idle;
        end if;
     
      when read_data =>
        output_state <= reg_data;
        
      when reg_data =>
        output_state <= increment_read_address;
      
      when increment_read_address =>
      
        -- For compatibility, the left and right channels are the even and odd channels, respectively,
        -- so when either the last even or the last odd channel is read then move to the read finish state
        if read_counter = 2*n_drivers - 2 then 
          output_state <= read_finish;
          
        -- Otherwise keep reading the data out
        else
          output_state <= read_data;
        end if;
        
      when read_finish =>
        output_state <= idle;
      
      when others => 
        output_state <= idle;
        
    end case;
    
  end if;
end process;

data_out_process : process(sys_clk,reset_n)
begin
  if reset_n = '0' then 
    read_address   <= (others => '0');
    read_counter <= (others => '0');
    rden <= '0';
  elsif rising_edge(sys_clk) then 
    case output_state is 
    
      -- When idle, reset the counters and the read signal
      when idle =>
        read_address   <= (others => '0');
        read_counter <= (others => '0');
        driver_counter <= (others => '0');
        read_all <= '0';
      
       when increment_read_address => 
        -- Increment the channel read counter by two and the driver counter by one
        read_counter    <= read_counter + 2;
        driver_counter  <= driver_counter + 1;

      when read_data =>
        -- Enable the read
        rden <= '1';
        
        -- Select the channel read address based on what the L/R select is
        if lrclk_r = '1' then 
          read_address <= std_logic_vector(read_counter(6 downto 0) + 1);
        else 
          read_address <= std_logic_vector(read_counter(6 downto 0) + 0);
        end if; 
        
      when reg_data => -- Merge this with the read_data state?
        -- Register the data and flip the read enable signal
        -- data_array_r(to_integer(driver_counter)) <= output_data_r; 
        if output_data_r(31) = '1' then 
          data_array_r(to_integer(driver_counter)) <= "11" & output_data_r(29 downto 0);
        else
          data_array_r(to_integer(driver_counter)) <= "00" & output_data_r(29 downto 0);
        end if;
        rden <= '0';
        
      when read_finish =>
        -- Indicate all the data has been read out
        read_all <= '1';
     
      when others => 
    
    end case;
  end if;
end process;

sdata_process : process(bclk_r,reset_n)
begin 
  if reset_n = '0' then 

  elsif falling_edge(bclk_r) then 
  
    -- On the last bitclock before the LR clock changes, load the new data
    if lrclk_counter = lrclk_div - 1 then 
      load_data <= '1';
      read_trigger <= '0';
      
    -- Load the new data into the register a few cycles before the final bit is transmitted
    elsif lrclk_counter = lrclk_div - 1 - read_ahead then 
      read_trigger <= '1';
    else
      read_trigger <= '0';
      load_data <= '0';
    end if;
  end if;
end process;

load_shift_generate: for shift_ind in max_drivers - 1 downto 0 generate
  serial_shift_map: Gen_Shift_Container
  port map (  clk => not bclk_r,
              input_data  => data_array_r(shift_ind),
              output_data => sdata_array_r(shift_ind),
              load => load_data
  );
end generate;

-- -- Delay the data by a single bitclock
-- follower_process : process(bclk_r)
-- begin 
  -- if rising_edge(bclk_r) then 
    -- sdata_follower_r <= sdata_array_r;
  -- end if;
-- end process;

-- Map the signals to the output ports
bclk_out  <= bclk_r;
lrclk_out <= lrclk_r;

serial_output_generate: for out_ind in max_drivers - 1 downto 0  generate
    sdata_out(out_ind) <= sdata_array_r(out_ind)(to_integer(lrclk_div - 1));
end generate;

end architecture rtl;























































