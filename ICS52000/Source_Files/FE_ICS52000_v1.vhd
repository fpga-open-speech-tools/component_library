----------------------------------------------------------------------------
--! @file FE_ICS52000.vhd
--! @brief 
--! @details  
--! @author Tyler Davis
--! @date 2020
--! @copyright Copyright 2020 Audio Logic
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

entity FE_ICS52000 is
  generic ( 
    avalon_data_width   : integer := 32;
    mic_data_width      : integer := 24;
    cfg_data_width      : integer := 16;
    ch_width            : integer := 6;
    n_mics              : integer := 16;
    n_clocks            : integer := 4;
    sck_multiplier      : integer := 64
  );
  
  port (
    sys_clk               : in  std_logic                     := '0';
    mic_clk_in            : in  std_logic                     := '0';
    reset_n               : in  std_logic                     := '0';
   
    mic_out_data          : out  std_logic_vector(avalon_data_width-1 downto 0) := (others => '0');
    mic_out_channel       : out  std_logic_vector(ch_width-1 downto 0)  := (others => '0');
    mic_out_error         : out  std_logic_vector(1 downto 0)  := (others => '0');
    mic_out_valid         : out  std_logic                     := '0';
    
    cfg_input_data        : in std_logic_vector(cfg_data_width-1 downto 0) := (others => '0');
    cfg_input_valid       : in std_logic := '0';
    cfg_input_error       : in std_logic_vector(1 downto 0) := (others => '0');
          
    mic_data_in           : in  std_logic_vector(n_mics-1 downto 0) := (others => '0');
    mic_ws_out            : out std_logic_vector(n_mics-1 downto 0) := (others => '0');
    mic_clk_out           : out std_logic_vector(n_clocks-1 downto 0) := (others => '0');
    
    mics_rdy              : out std_logic
  );
end entity FE_ICS52000;

architecture rtl of FE_ICS52000 is

component Serial2Parallel_32bits is
	port
	(
		clock		: in std_logic ;
		shiftin : in std_logic ;
		q		    : out std_logic_vector (31 downto 0)
	);
end component;

component mic_array_startup is
  generic 
  (
    clk_frequency                 : integer := 24576000;
    startup_time_ms               : integer := 50;
    sck_cycles_until_data_valid   : integer := 262144;
    cycles_per_frame              : integer := 64
  );
  port 
  (
    sck         : in  std_logic;
    rst         : in  std_logic;
    data_valid  : out std_logic;
    ws          : out std_logic
  );
end component;

-- Mic array signals 
signal mic_ws_out_r   : std_logic_vector(n_mics-1 downto 0) := (others => '0');
signal startup_valid  : std_logic_vector(n_mics-1 downto 0) := (others => '0');

-- Deserialization signals
signal read_bits              : integer range 0 to avalon_data_width := 0;
signal read_word_bits         : integer range 0 to avalon_data_width := 0;

signal cfg_data_r             : std_logic_vector(cfg_data_width-1 downto 0) := (others => '0');

signal send_valid             : std_logic := '0';
signal busy                   : std_logic := '0';
signal read_mics              : std_logic := '0';

-- Control signals
signal start_shifting         : std_logic := '0';
signal end_shifting           : std_logic := '0';
signal shift_busy             : std_logic := '0';

-- Avalon streaming signals
type mic_array_data is array (n_mics-1 downto 0) of std_logic_vector(avalon_data_width-1 downto 0);

-- Workaround for a memory initialization error associated with defining an array
signal mic_data_r             : mic_array_data := (others => (others => '0'));
signal mic_out_data_r         : std_logic_vector(avalon_data_width-1 downto 0) := (others => '0');
signal deser_data             : mic_array_data := (others => (others => '0'));
signal cfg_channel_counter    : integer range 0 to n_mics := 0;
signal channel_counter        : integer range 0 to n_mics := 0;
signal mic_channel_r          : std_logic_vector(ch_width-1 downto 0) := (others => '0');
signal mic_out_valid_r        : std_logic := '0';

signal high_z_filler          : std_logic_vector(avalon_data_width-mic_data_width-1 downto 0) := (others => '0');

-- Create the states for the deserialzier state machine
type deser_state is (idle, mic_data_wait, read_mic_data, valid_pulse);

-- Enable recovery from illegal state
attribute syn_encoding        : string;
attribute syn_encoding of deser_state : type is "safe";

signal cur_sdi_state : deser_state := idle;

type mic_valid is (idle, increment_counter, pulse, low_wait);

-- Enable recovery from illegal state
attribute syn_encoding of mic_valid : type is "safe";

signal mic_valid_state : mic_valid := idle;

begin 

deserializer_generate : for deser_ind in n_mics-1 downto 0 generate
  deserializer_map : Serial2Parallel_32bits
  port map 
  (
    clock     => mic_clk_in,
    shiftin   => mic_data_in(deser_ind),
    q         => deser_data(deser_ind)
  );
end generate;

startup_generate : for startup_ind in n_mics -1 downto 0 generate
  mic_startup_map : mic_array_startup
  generic map 
  (
    clk_frequency                 => 24576000,
    startup_time_ms               => 20,
    sck_cycles_until_data_valid   => 262144,
    cycles_per_frame              => 64
  )
  port map
  (
    sck         => mic_clk_in,
    rst         => not reset_n,
    data_valid  => startup_valid(startup_ind),
    ws          => mic_ws_out_r(startup_ind)
  );
end generate;

clk_generate : for clk_ind in n_clocks-1 downto 0 generate
  mic_clk_out(clk_ind) <= mic_clk_in;
end generate;

bit_counter_process : process(mic_clk_in,reset_n)
begin 
  if reset_n = '0' then 
    read_bits <= 0;
    
  elsif rising_edge(mic_clk_in) then 
    -- If the input state machine is idle, don't count the bits coming into the component
    if cur_sdi_state = idle then 
      read_bits <= 0;
      
    -- When the counter reaches the current number of expected bits, reset it
    elsif read_bits = read_word_bits - 1 then 
      read_bits <= 0;
    else
    -- Otherwise, increment the bit counter
      read_bits <= read_bits + 1;
    end if;
  end if;
end process;

data_in_transition_process : process(mic_clk_in, reset_n)
begin 
  if reset_n = '0' then 
  
  elsif rising_edge(mic_clk_in) then 
    case cur_sdi_state is
      when idle => 
        -- Since all mics are identical, the setup time should be as well.  Therefore, wait for 
        -- one mic to become ready then start reading data
        if mic_ws_out_r(0) = '1' then
          cur_sdi_state <= mic_data_wait;
          
        -- Otherwise, remain idle
        else
          cur_sdi_state <= idle;
        end if;
        
      when mic_data_wait =>
        -- Once on the last shift bit, move to the read state
        if read_bits = read_word_bits - 2 then 
          cur_sdi_state <= read_mic_data;
          
        -- Otherwise, keep waiting for the data to shift
        else
          cur_sdi_state <= mic_data_wait;
        end if;
        
      when read_mic_data =>
        -- Once the microphone data has been read, start transferring data over the Avalon interfaces
          cur_sdi_state <= valid_pulse;
      
      when valid_pulse =>
        cur_sdi_state <= idle;
        
      when others =>
    end case;
  end if;
end process;

data_in_process : process(mic_clk_in, reset_n)
begin 
  if reset_n = '0' then 
  
  elsif rising_edge(mic_clk_in) then 
    case cur_sdi_state is
      when idle => 
        send_valid <= '0';
        
      when mic_data_wait =>
        read_word_bits <= avalon_data_width;
        
      when read_mic_data =>
        -- "Shift in" the microphone data
        mic_data_r <= deser_data;

      when valid_pulse =>
        -- Signal the valid state machines to start transferring data
        send_valid <= '1';
               
      when others =>
    end case;
  end if;
end process;

-- Process to control the output data transfer state machine
mic_data_valid_transition_process : process(sys_clk,reset_n)
begin
  if reset_n = '0' then 
  elsif rising_edge(sys_clk) then 
    case mic_valid_state is 
    
      when idle => 
        -- Once the data is ready to send, send a valid pulse at the system clock frequency (Avalon streaming)
        if send_valid = '1' then 
          mic_valid_state <= pulse;
          
        -- Otherwise stay idle
        else
          mic_valid_state <= idle;
        end if;
      
      when increment_counter =>
        -- Immediately send another pulse
        mic_valid_state <= pulse;
          
      
      when pulse =>
        -- If the number of channels sent equals the number of mics, move to the waiting state
        if channel_counter = n_mics - 1 then 
          mic_valid_state <= low_wait;
          
        -- Otherwise, increment the channel
        else
          mic_valid_state <= increment_counter;
        end if;
        
      when low_wait =>
        -- If the send signal has gone low (slower clock frequency), transition to the idle state
        if send_valid = '0' then 
          mic_valid_state <= idle;
          
        -- Otherwise, wait continue waiting
        else
          mic_valid_state <= low_wait;
        end if;
      
      when others => 
    
    end case;
  end if;
end process;

mic_data_valid_process : process(sys_clk,reset_n)
begin
  if reset_n = '0' then 
    mic_out_valid_r <= '0';
    channel_counter <= 0;
  elsif rising_edge(sys_clk) then 
    case mic_valid_state is 
    
      when idle => 
        -- Reset the channel counter
        channel_counter   <= 0;
        cfg_channel_counter <= 0;
              
      when increment_counter =>
        -- Increment the channel counter and disable the valid
        channel_counter   <= channel_counter + 1;
        mic_out_valid_r   <= '0';
        if cfg_data_r(channel_counter) = '1' then 
          cfg_channel_counter <= cfg_channel_counter + 1;
        else
          cfg_channel_counter <= cfg_channel_counter;
        end if;
        
      when pulse =>
        -- Set the channel and pulse the valid 
        mic_out_valid_r   <= cfg_data_r(channel_counter);
        mic_out_data_r    <= high_z_filler & mic_data_r(channel_counter)(avalon_data_width-1 downto avalon_data_width-mic_data_width);
        mic_channel_r     <= std_logic_vector(to_unsigned(cfg_channel_counter,mic_channel_r'length));
        
      when low_wait =>
        -- Disable the valid signal
        mic_out_valid_r   <= '0';
        
      when others => 
    
    end case;
  end if;
end process;

cfg_input_process : process(sys_clk,reset_n)
begin 
  if reset_n = '0' then 
    cfg_data_r <= (others => '1');
  elsif rising_edge(sys_clk) then 
    if cfg_input_valid = '1' then 
      cfg_data_r <= cfg_input_data;
    else
      cfg_data_r <= cfg_data_r;
    end if;
  end if;
end process;

mic_ws_out <= mic_ws_out_r;

mic_out_data    <= mic_out_data_r;
mic_out_channel <= mic_channel_r;
mic_out_valid   <= mic_out_valid_r;

mics_rdy <= startup_valid(0);

end architecture rtl;























































