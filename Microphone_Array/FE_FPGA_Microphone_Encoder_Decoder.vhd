----------------------------------------------------------------------------
--! @file FE_FPGA_Microphone_Encoder_Decoder.vhd
--! @brief FPGA microphone array encoder/decoder component
--! @details  This component reads microphone, temperature, pressure, and humidity data from the CPLD microphone array
--            component and transmits commands over serial lines.
--! @author Tyler Davis
--! @date 2020
--! @copyright Copyright 2020 Flat Earth Inc
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

entity FE_FPGA_Microphone_Encoder_Decoder is
  generic ( 
    avalon_data_width   : integer := 32;
    mic_data_width      : integer := 24;
    bme_data_width      : integer := 96;
    rgb_data_width      : integer := 16;
    cfg_data_width      : integer := 16;
    ch_width            : integer := 4;
    n_mics              : integer := 16;
    n_mics_max          : integer := 16
  );
  
  port (
    sys_clk               : in  std_logic                     := '0';
    serial_clk            : in  std_logic                     := '0';
    reset_n               : in  std_logic                     := '0';
    
    busy_out              : out std_logic := '0';
        
    bme_out_data          : out std_logic_vector(bme_data_width-1 downto 0) := (others => '0');
    bme_out_valid         : out std_logic := '0';
    bme_out_error         : out std_logic_vector(1 downto 0) := (others => '0');
   
    mic_out_data          : out  std_logic_vector(avalon_data_width-1 downto 0) := (others => '0');
    mic_out_channel       : out  std_logic_vector(ch_width-1 downto 0)  := (others => '0');
    mic_out_error         : out  std_logic_vector(1 downto 0)  := (others => '0');
    mic_out_valid         : out  std_logic                     := '0';
      
    rgb_input_data        : in std_logic_vector(rgb_data_width-1 downto 0) := (others => '0');
    rgb_input_valid       : in std_logic := '0';
    rgb_input_error       : in std_logic_vector(1 downto 0) := (others => '0');
    
    cfg_input_data        : in std_logic_vector(cfg_data_width-1 downto 0) := (others => '0');
    cfg_input_valid       : in std_logic := '0';
    cfg_input_error       : in std_logic_vector(1 downto 0) := (others => '0');
          
    serial_data_in        : in std_logic;
    serial_data_out       : out std_logic                      := '0';
    serial_clk_out        : out  std_logic                     := '0'
  );
    
end entity FE_FPGA_Microphone_Encoder_Decoder;

architecture rtl of FE_FPGA_Microphone_Encoder_Decoder is

-- Instantiate the component that shifts the data
component Generic_Shift_Container
  generic (   
    data_width : integer := 8
  );
  port (
  clk         : in  std_logic;
  input_data  : in  std_logic_vector(data_width-1 downto 0);
  output_data : out std_logic_vector(data_width-1 downto 0);
  load        : in  std_logic
  );
end component;

-- Data byte width definitions
signal MAX_SDATA_SIZE             : integer := 96;
signal header_byte_width          : integer := 4;
signal packet_cntr_byte_width     : integer := 4;
signal n_mic_byte_width           : integer := 1;
signal temp_byte_width            : integer := 4;
signal humid_byte_width           : integer := 4;
signal pressure_byte_width        : integer := 4;
signal mic_byte_width             : integer := 3;
signal cfg_byte_width             : integer := 8;
signal rgb_byte_width             : integer := 2;
signal packet_number_byte_width   : integer := 4;
                                  
-- BME word division definitions  
signal temp_byte_location         : integer := 12;
signal humid_byte_location        : integer := 8;
signal pressure_byte_location     : integer := 4;
                                  
-- Packet DATA_HEADER ID          
signal header_width               : integer := 32;
signal DATA_HEADER                : std_logic_vector(header_width-1 downto 0) := x"43504C44";
signal CMD_HEADER                 : std_logic_vector(header_width-1 downto 0) := x"46504741";
                                  
-- Shift state signals            
signal shift_data                 : std_logic_vector(MAX_SDATA_SIZE-1 downto 0) := (others => '0');
signal shift_data_in              : std_logic_vector(7 downto 0) := (others => '0');
signal shift_data_out             : std_logic_vector(7 downto 0) := (others => '0');
signal byte_counter               : integer range 0 to 8 := 0;
signal n_bytes                    : integer range 0 to 8 := 0;
signal bit_counter                : integer range 0 to 7 := 0;
signal shift_out                  : std_logic;
signal shift_en_n                 : std_logic := '0';
signal load_data                  : std_logic := '0';
signal packet_number              : unsigned(31 downto 0) := (others => '0');
signal sdo_mics                   : integer range 0 to 64 := 16;
constant shift_width              : integer := 8;
                                  
-- Deserialization signals        
signal parallel_data_r            : std_logic_vector(MAX_SDATA_SIZE-1 downto 0) := (others => '0');
signal header_found               : std_logic := '0';
signal read_bits                  : integer range 0 to MAX_SDATA_SIZE := 0;
signal read_word_bits             : integer range 0 to MAX_SDATA_SIZE := 0;
signal sdo_mics_r                 : integer range 0 to 32 := 16;
signal cfg_data_r                 : std_logic_vector(8*cfg_byte_width-1 downto 0) := (others => '0');
signal cfg_out_valid_r            : std_logic := '0';
signal rgb_data_r                 : std_logic_vector(8*rgb_byte_width-1 downto 0) := (others => '0');
signal rgb_out_valid_r            : std_logic := '0';
signal cur_mic                    : integer range 0 to n_mics_max := 0;
                                  
signal send_valid                 : std_logic := '0';
signal busy                       : std_logic := '0';
signal read_mics                  : std_logic := '0';
                                  
-- Control signals                
signal start_shifting             : std_logic := '0';
signal end_shifting               : std_logic := '0';
signal shift_busy                 : std_logic := '0';

-- Avalon streaming signals
type mic_array_data is array (n_mics_max-1 downto 0) of std_logic_vector(mic_data_width-1 downto 0);

-- Workaround for a memory initialization error associated with defining an array
-- Assignments -> Device -> Device and Pin Options -> Configuration -> Configuration Mode: Single uncompressed image with Memory Initialization
signal mic_data_r             : mic_array_data := (others => (others => '0'));
signal mic_data_out_r         : std_logic_vector(mic_data_width-1 downto 0) := (others => '0');
signal channel_counter        : integer range 0 to n_mics_max := 0;
signal mic_channel_r          : std_logic_vector(ch_width-1 downto 0) := (others => '0');
signal mic_out_valid_r        : std_logic := '0';

signal bme_data_r             : std_logic_vector(bme_data_width-1 downto 0) := (others => '0');
signal bme_out_valid_r        : std_logic := '0';


-- Create states for the output state machine
type serializer_state is (  idle, load_header, load_n_mics, load_cfg, load_rgb,
                            load_shift_reg, shift_wait );
-- Enable recovery from illegal state
attribute syn_encoding        : string;
attribute syn_encoding of serializer_state : type is "safe";

signal cur_sdo_state          : serializer_state := idle;
signal next_sdo_state         : serializer_state := idle;


-- Create the states for the deserialzier state machine
type deser_state is (idle, read_packet_number, read_n_mics, read_pressure, read_temp, read_humid, read_mic_data, valid_pulse);

-- Enable recovery from illegal state
attribute syn_encoding of deser_state : type is "safe";

signal cur_sdi_state : deser_state := idle;

type bme_valid is (idle, pulse, low_wait);

-- Enable recovery from illegal state
attribute syn_encoding of bme_valid : type is "safe";

signal bme_valid_state : bme_valid := idle;

type mic_valid is (idle, increment_counter, pulse, low_wait);

-- Enable recovery from illegal state
attribute syn_encoding of mic_valid : type is "safe";

signal mic_valid_state : mic_valid := idle;

begin 

serial_shift_map: Generic_Shift_Container    
generic map (
  data_width => shift_width
)
port map (  
  clk => serial_clk,
  input_data  => shift_data_in,
  output_data => shift_data_out,
  load => load_data
);

-- Process to start the bit shifting 
shift_start_process : process(serial_clk,reset_n)
begin 
  if reset_n = '0' then 
    start_shifting <= '0';
    shift_busy <= '0';
  elsif rising_edge(serial_clk) then 
  
    -- When the first data packet is received, start shifting the DATA_HEADER out
    if cfg_input_valid = '1' then -- TODO: Find a better start condition
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

data_out_transition_process : process(serial_clk,reset_n)
begin
  if reset_n = '0' then 

  elsif rising_edge(serial_clk) then 
    case cur_sdo_state is 
      when idle => 
        -- Wait for the start_shifting signal to load the header
        if start_shifting = '1' then 
          cur_sdo_state <= load_header;
        else  
          cur_sdo_state <= idle;
        end if;
        
      when load_header =>
      -- Transition to the shift state, then set the next state to load
        cur_sdo_state <= load_shift_reg;
        next_sdo_state <= load_n_mics;
        
      when load_n_mics =>
      -- Transition to the shift state, then set the next state to load
        cur_sdo_state <= load_shift_reg;
        next_sdo_state <= load_cfg;
        
      when load_cfg =>
      -- Transition to the shift state, then set the next state to load
        cur_sdo_state <= load_shift_reg;
        next_sdo_state <= load_rgb;
        
      when load_rgb =>
      -- Transition to the shift state, then set the next state to load
        cur_sdo_state <= load_shift_reg;
        next_sdo_state <= idle;
      
      when load_shift_reg =>
        -- Immediately transition to the wait state
        cur_sdo_state <= shift_wait;
      
      when shift_wait =>
        -- If the specified number of bytes have been sent, transition to the next load state before the shift register 
        -- "empties"
        if byte_counter = n_bytes and bit_counter = shift_width - 3 then 
          cur_sdo_state <= next_sdo_state;
          
        -- If there are still more bytes to load, load the next byte into the register
        elsif byte_counter < n_bytes and bit_counter = shift_width - 2 then 
          cur_sdo_state <= load_shift_reg;
          
        -- Otherwise, stay in the wait state
        else
          cur_sdo_state <= shift_wait;
        end if;

      when others => 
    
    end case;
  end if;
end process;

data_out_process : process(serial_clk,reset_n)
begin
  if reset_n = '0' then 

  elsif rising_edge(serial_clk) then 
    case cur_sdo_state is 
      when idle => 
        -- Reset the counters and signal the component is no longer busy
        bit_counter   <= 0;
        --mic_counter <= 0;
        busy <= '0';
        
      when load_header =>
        -- Reset the counters and signal the component is no longer busy
        shift_data(header_width-1 downto 0) <= CMD_HEADER;
        n_bytes <= header_byte_width;
        byte_counter <= 0;
        
      when load_n_mics =>
        -- Load the number of microphones into the shift register
        shift_data(7 downto 0) <= std_logic_vector(to_unsigned(n_mics,8));
        n_bytes <= n_mic_byte_width;
        byte_counter <= 0;
        
      when load_cfg =>
        -- Load the mic configuration into the shift register
        shift_data(8*cfg_byte_width-1 downto 0) <= cfg_data_r;
        n_bytes <= cfg_byte_width;
        byte_counter <= 0;
        
      when load_rgb =>
        -- Load the RGB LED settings into the shift register
        shift_data(8*rgb_byte_width-1 downto 0) <= rgb_data_r;
        n_bytes <= rgb_byte_width;
        byte_counter <= 0;
      
      when load_shift_reg =>
        -- Reset the bit counter and increment the byte counter
        bit_counter <= 0;
        byte_counter <= byte_counter + 1;
        
        -- Load the next set of data into the shift component
        shift_data_in <= shift_data(8*(n_bytes-byte_counter)-1 downto 8*(n_bytes-byte_counter-1));
        load_data <= '1';
        
        -- Signal the component is busy
        busy <= '1';
      
      when shift_wait =>
        -- Increment the bit counter and reset the shift component load signal
        bit_counter <= bit_counter + 1;
        load_data <= '0';
      
      when others => 
    
    end case;
  end if;
end process;


shift_process: process(serial_clk,reset_n)
begin
  if reset_n = '0' then 
    parallel_data_r <= (others => '0');
  elsif rising_edge(serial_clk) then 
      parallel_data_r <= parallel_data_r(MAX_SDATA_SIZE-2 downto 0) & serial_data_in;
  end if;
end process;

bit_counter_process : process(serial_clk,reset_n)
begin 
  if reset_n = '0' then 
    read_bits <= 0;
    
  elsif rising_edge(serial_clk) then 
    -- If the input state machine is idle, don't count the bits coming into the component
    if cur_sdi_state = idle then 
      read_bits <= 0;
      
    -- When the counter reaches the current number of expected bits, reset it
    elsif read_bits = read_word_bits - 1 then 
      read_bits <= 0;
      
      -- Also increment the mic counter if reading microphone data
      if read_mics = '1' then 
        cur_mic <= cur_mic + 1;
      else
        cur_mic <= 0;
      end if;
    else
      
    -- Otherwise, increment the bit counter
      read_bits <= read_bits + 1;
    end if;
  end if;
end process;

data_in_transition_process : process(serial_clk, reset_n)
begin 
  if reset_n = '0' then 
  
  elsif rising_edge(serial_clk) then 
    case cur_sdi_state is
      when idle => 
        -- If the header has been read, transition to reading the number of mics
        if parallel_data_r(8*header_byte_width-1 downto 0) = DATA_HEADER then
          cur_sdi_state <= read_packet_number;
          
        -- Otherwise, remain idle
        else
          cur_sdi_state <= idle;
        end if;
        
      when read_packet_number =>
        -- Once the packet number has been read, read the number of microphones
        if read_bits = read_word_bits - 1 then 
          cur_sdi_state <= read_n_mics;
        else
          cur_sdi_state <= read_packet_number;
        end if;
        
      when read_n_mics =>
        -- Once the number of microphones has been read, read the temperature data
        if read_bits = read_word_bits - 1 then 
          cur_sdi_state <= read_temp;
        else
          cur_sdi_state <= read_n_mics;
        end if;
      
      when read_temp =>
        -- Once the temperature data has been read, read the pressure data
        if read_bits = read_word_bits - 1 then 
          cur_sdi_state <= read_pressure;
        else
          cur_sdi_state <= read_temp;
        end if;
      
      when read_pressure =>
        -- Once the pressure data has been read, read the humidity data
        if read_bits = read_word_bits - 1 then 
          cur_sdi_state <= read_humid;
        else
          cur_sdi_state <= read_pressure;
        end if;
      
      when read_humid =>
        -- Once the humidity data has been read, read the microphone data
        if read_bits = read_word_bits - 1 then 
          cur_sdi_state <= read_mic_data;
        else
          cur_sdi_state <= read_humid;
        end if;
      
      when read_mic_data =>
        -- Once the microphone data has been read, start transferring data over the Avalon interfaces
        if read_bits = read_word_bits - 1 and n_mics - 1 = cur_mic then 
          cur_sdi_state <= valid_pulse;
        else
          cur_sdi_state <= read_mic_data;
        end if;
      
      when valid_pulse =>
        cur_sdi_state <= idle;
        
      when others =>
    end case;
  end if;
end process;

data_in_process : process(serial_clk, reset_n)
begin 
  if reset_n = '0' then 
  
  elsif rising_edge(serial_clk) then 
    case cur_sdi_state is
      when idle => 
        send_valid <= '0';

      when read_packet_number =>
        -- "Shift in" the packet number
        packet_number <= unsigned(parallel_data_r(8*packet_number_byte_width-1 downto 0));
        read_word_bits <= 8*packet_number_byte_width;

      when read_n_mics =>
        -- "Shift in" the number of microphones
        sdo_mics_r <= to_integer(unsigned(parallel_data_r(8*n_mic_byte_width-1 downto 0)));
        read_word_bits <= 8*n_mic_byte_width;

      when read_pressure =>
        -- "Shift in" the pressure data
        bme_data_r(8*pressure_byte_location-1 downto 8*(pressure_byte_location-pressure_byte_width)) <= parallel_data_r(8*pressure_byte_width-1 downto 0);
        read_word_bits <= 8*pressure_byte_width;
      
      when read_temp =>
        -- "Shift in" the temperature data
        bme_data_r(8*temp_byte_location-1 downto 8*(temp_byte_location-temp_byte_width)) <= parallel_data_r(8*temp_byte_width-1 downto 0);
        read_word_bits <= 8*temp_byte_width;
      
      when read_humid =>
        -- "Shift in" the humidity data
        bme_data_r(8*humid_byte_location-1 downto 8*(humid_byte_location-humid_byte_width)) <= parallel_data_r(8*humid_byte_width-1 downto 0);
        read_word_bits <= 8*humid_byte_width;
        
      when read_mic_data =>
        -- "Shift in" the microphone data
        read_mics <= '1';
        mic_data_r(cur_mic) <= parallel_data_r(8*mic_byte_width-1 downto 0);
        read_word_bits <= 8*mic_byte_width;

      when valid_pulse =>
        -- Signal the valid state machines to start transferring data
        read_mics <= '0';
        send_valid <= '1';
               
      when others =>
    end case;
  end if;
end process;

bme_data_valid_transition_process : process(sys_clk,reset_n)
begin
  if reset_n = '0' then 
    bme_valid_state <= idle;
  elsif rising_edge(sys_clk) then 
    case bme_valid_state is 
    
      when idle =>
        -- Once the data has been read in, send a valid pulse at the system clock frequency (Avalon streaming)
        if send_valid = '1' then 
          bme_valid_state <= pulse;
          
        -- Otherwise, stay idle
        else
          bme_valid_state <= idle;
        end if;
      
      when pulse => 
        -- Transition to the wait state
        bme_valid_state <= low_wait;
        
      when low_wait =>
        -- When "send" signal goes low (slower clock frequency), go idle again
        if send_valid = '0' then 
          bme_valid_state <= idle;
          
        -- Otherwise, wait for the valid to go low
        else
          bme_valid_state <= low_wait;
        end if;
        
      when others =>    
    end case;
  end if;
  
end process; 

bme_data_valid_process : process(sys_clk,reset_n)
begin
  if reset_n = '0' then 
      bme_out_valid_r <= '0';
  elsif rising_edge(sys_clk) then 
    case bme_valid_state is 
      
      when idle =>
      -- Do nothing
      when pulse => 
        -- Pulse the valid Avalon streaming signal
        bme_out_valid_r <= '1';
        
      when low_wait =>
        bme_out_valid_r <= '0';
        
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
        if channel_counter = n_mics_max - 1 then 
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
              
      when increment_counter =>
        -- Increment the channel counter and disable the valid
        channel_counter   <= channel_counter + 1;
        mic_out_valid_r   <= '0';
      
      when pulse =>
        -- Set the channel and pulse the valid 
        mic_channel_r         <= std_logic_vector(to_unsigned(channel_counter,mic_channel_r'length));
        mic_data_out_r        <= mic_data_r(channel_counter);
        mic_out_valid_r       <= '1';
        
      when low_wait =>
        -- Disable the valid signal
        mic_out_valid_r   <= '0';
        
      when others => 
    
    end case;
  end if;
end process;



-- Map the RJ45 signals to the output ports
serial_data_out <= shift_data_out(shift_width-1);
serial_clk_out  <= serial_clk;

-- Map the busy signal 
busy_out <= busy;

bme_out_data    <= bme_data_r;
bme_out_valid   <= bme_out_valid_r; 

mic_out_data(8*mic_byte_width-1 downto 0)   <= mic_data_out_r;
mic_out_channel                             <= mic_channel_r;
mic_out_valid                               <= mic_out_valid_r;

end architecture rtl;























































