----------------------------------------------------------------------------
--! @file FE_Microphone_Encoder_Decoder.vhd
--! @brief Speaker array encoder component
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

entity FE_Microphone_Encoder_Decoder is
  generic ( 
    avalon_data_width   : integer := 32;
    mic_data_width      : integer := 24;
    bme_data_width      : integer := 64;
    rgb_data_width      : integer := 16;
    cfg_data_width      : integer := 72;
    ch_width            : integer := 4;
    n_mics              : integer := 16
  );
  
  port (
    sys_clk               : in  std_logic                     := '0';
    reset_n               : in  std_logic                     := '0';
    
    busy_out              : out std_logic := '0';
        
    bme_input_data        : in std_logic_vector(bme_data_width-1 downto 0) := (others => '0');
    bme_input_valid       : in std_logic := '0';
    bme_input_error       : in std_logic_vector(1 downto 0) := (others => '0');
      
    mic_input_data        : in  std_logic_vector(avalon_data_width-1 downto 0) := (others => '0');
    mic_input_channel     : in  std_logic_vector(ch_width-1 downto 0)  := (others => '0');
    mic_input_error       : in  std_logic_vector(1 downto 0)  := (others => '0');
    mic_input_valid       : in  std_logic                     := '0';
      
    rgb_out_data          : out std_logic_vector(rgb_data_width-1 downto 0) := (others => '0');
    rgb_out_valid         : out std_logic := '0';
    rgb_out_error         : out std_logic_vector(1 downto 0) := (others => '0');

    cfg_out_data          : out std_logic_vector(cfg_data_width-1 downto 0) := (others => '0');
    cfg_out_valid         : out std_logic := '0';
    cfg_out_error         : out std_logic_vector(1 downto 0) := (others => '0');
          
    led_sd                : out std_logic                     := '0';
    led_ws                : out std_logic                     := '0';
          
    serial_data_out       : out std_logic;
    serial_data_in        : in std_logic                      := '0';
    serial_clk_in         : in  std_logic                     := '0';
    
    debug_out             : out std_logic_vector(7 downto 0) := (others => '0')
  );
    
end entity FE_Microphone_Encoder_Decoder;

architecture rtl of FE_Microphone_Encoder_Decoder is

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

-- Packet DATA_HEADER ID
signal header_width : integer := 32;
signal DATA_HEADER  : std_logic_vector(header_width-1 downto 0) := x"43504C44";
signal CMD_HEADER   : std_logic_vector(header_width-1 downto 0) := x"46504741";

-- Shift state signals
signal shift_data             : std_logic_vector(31 downto 0) := (others => '0');
signal shift_data_in          : std_logic_vector(7 downto 0) := (others => '0');
signal shift_data_out         : std_logic_vector(7 downto 0) := (others => '0');
signal byte_counter           : integer range 0 to 4 := 0;
signal byte_counter_follower  : integer range 0 to 4 := 0;
signal n_bytes                : integer range 0 to 4 := 0;
signal bit_counter            : integer range 0 to 7 := 0;
signal mic_counter            : integer range 0 to 64 := 0;
signal mic_counter_follower   : integer range 0 to 64 := 0;
signal shift_out              : std_logic;
signal shift_en_n             : std_logic := '0';
signal load_data              : std_logic := '0';
signal packet_counter         : unsigned(31 downto 0) := (others => '0');
constant shift_width          : integer := 8;


-- Deserialization signals
signal parallel_data_r  : std_logic_vector(31 downto 0) := (others => '0');
signal header_found     : std_logic := '0';
signal read_bits        : integer range 0 to 32 := 0;
signal read_word_bits   : integer range 0 to 32 := 0;

signal busy : std_logic := '0';

-- Control signals
signal start_shifting : std_logic := '0';
signal end_shifting   : std_logic := '0';
signal shift_busy     : std_logic := '0';

-- Avalon streaming signals
type mic_array_data is array (n_mics-1 downto 0) of std_logic_vector(mic_data_width-1 downto 0);
signal mic_input_data_r : mic_array_data := (others => (others => '0'));
signal bme_input_data_r : std_logic_vector(bme_data_width-1 downto 0) := (others => '0');

-- Data byte width definitions
signal header_byte_width        : integer := 4;
signal packet_cntr_byte_width   : integer := 4;
signal n_mic_byte_width         : integer := 1;
signal temp_byte_width          : integer := 3;
signal humid_byte_width         : integer := 2;
signal pressure_byte_width      : integer := 3;
signal mic_byte_width           : integer := 3;

-- BME word division definitions
signal temp_byte_location       : integer := 8;
signal humid_byte_location      : integer := 5;
signal pressure_byte_location   : integer := 3;

-- Create states for the output state machine
type serializer_state is (  idle, load_header, load_packet_number, load_n_mics, load_temp, load_pressure, load_humidity,
                      load_mics, load_shift_reg, shift_wait );
-- Enable recovery from illegal state
attribute syn_encoding : string;
attribute syn_encoding of serializer_state : type is "safe";

signal cur_state  : serializer_state := idle;
signal next_state : serializer_state := idle;


-- Create the states for the deserialzier state machine
type deser_state is (idle, read_mics, read_enable, read_rgb);

-- Enable recovery from illegal state
attribute syn_encoding of deser_state : type is "safe";

begin 

serial_shift_map: Generic_Shift_Container    
generic map (
  data_width => shift_width
)
port map (  
  clk => serial_clk_in,
  input_data  => shift_data_in,
  output_data => shift_data_out,
  load => load_data
);

-- Process to push the data into the FIFO
data_in_process : process(sys_clk,reset_n)
begin 
  if reset_n = '0' then 
    mic_input_data_r <= (others => (others => '0'));
  elsif rising_edge(sys_clk) then 
    -- Accept new data only when the valid is asserted
    if mic_input_valid = '1' then 
      mic_input_data_r(to_integer(unsigned(mic_input_channel))) <= mic_input_data;
      
    -- Otherwise, reset the write enable and keep the current data
    else
      mic_input_data_r <= mic_input_data_r;
    end if;
  end if;
end process;

-- Process to start the bit shifting 
shift_start_process : process(serial_clk_in,reset_n)
begin 
  if reset_n = '0' then 
    start_shifting <= '0';
    shift_busy <= '0';
  elsif rising_edge(serial_clk_in) then 
  
    -- When the first data packet is received, start shifting the DATA_HEADER out
    if mic_input_channel(4 downto 0) = "00000" then --n_drivers then -- Note: SignalTap seems to mess with the comparison...
      start_shifting <= '1';
      packet_counter <= packet_counter + 1;
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

data_out_transition_process : process(serial_clk_in,reset_n)
begin
  if reset_n = '0' then 

  elsif rising_edge(serial_clk_in) then 
    case cur_state is 
      when idle => 
        if start_shifting = '1' then 
          cur_state <= load_header;
        else  
          cur_state <= idle;
        end if;
        
      when load_header =>
        cur_state <= load_shift_reg;
        next_state <= load_packet_number;
        
      when load_packet_number =>
        cur_state <= load_shift_reg;
        next_state <= load_n_mics;
        
      when load_n_mics =>
        cur_state <= load_shift_reg;
        next_state <= load_temp;
        
      when load_temp =>
        cur_state <= load_shift_reg;
        next_state <= load_pressure;
        
      when load_pressure =>
        cur_state <= load_shift_reg;
        next_state <= load_humidity;
        
      when load_humidity =>
        cur_state <= load_shift_reg;
        next_state <= load_mics;
        
      when load_mics =>
        cur_state <= load_shift_reg;
        if mic_counter - 1 < n_mics then 
          next_state <= load_mics;
        else
          next_state <= idle;
        end if;
      
      when load_shift_reg =>
        cur_state <= shift_wait;
      
      when shift_wait =>
        if byte_counter = n_bytes and bit_counter = shift_width - 3 then 
          cur_state <= next_state;
        elsif byte_counter < n_bytes and bit_counter = shift_width - 2 then 
          cur_state <= load_shift_reg;
        else
          cur_state <= shift_wait;
        end if;

      when others => 
    
    end case;
  end if;
end process;

data_out_process : process(serial_clk_in,reset_n)
begin
  if reset_n = '0' then 

  elsif rising_edge(serial_clk_in) then 
    case cur_state is 
      when idle => 
        bit_counter   <= 0;
        mic_counter <= 0;
        busy <= '0';
        
      when load_header =>
        shift_data <= DATA_HEADER;
        n_bytes <= header_byte_width;
        byte_counter <= 0;
        
      when load_packet_number =>
        shift_data <= std_logic_vector(packet_counter);
        n_bytes <= packet_cntr_byte_width;
        byte_counter <= 0;
        
      when load_n_mics =>
        shift_data(7 downto 0) <= std_logic_vector(to_unsigned(n_mics,8));
        n_bytes <= n_mic_byte_width;
        byte_counter <= 0;
        
      when load_temp =>
        shift_data(8*temp_byte_width-1 downto 0) <= bme_input_data_r(8*temp_byte_location-1 downto 8*(temp_byte_location-temp_byte_width));
        n_bytes <= temp_byte_width;
        byte_counter <= 0;
        
      when load_pressure =>
        shift_data(8*pressure_byte_width-1 downto 0) <= bme_input_data_r(8*pressure_byte_location-1 downto 8*(pressure_byte_location-pressure_byte_width));
        n_bytes <= pressure_byte_width;
        byte_counter <= 0;
        
      when load_humidity =>
        shift_data(8*humid_byte_width-1 downto 0) <= bme_input_data_r(8*humid_byte_location-1 downto 8*(humid_byte_location-humid_byte_width));
        n_bytes <= humid_byte_width;
        byte_counter <= 0;
      
      when load_mics =>
        mic_counter <= mic_counter + 1;
        shift_data(8*mic_byte_width-1 downto 0) <= mic_input_data_r(mic_counter_follower);
        n_bytes <= mic_byte_width;
        byte_counter <= 0;
      
      when load_shift_reg =>
        mic_counter_follower <= mic_counter;
        bit_counter <= 0;
        byte_counter <= byte_counter + 1;
        shift_data_in <= shift_data(8*(n_bytes-byte_counter)-1 downto 8*(n_bytes-byte_counter-1));
        load_data <= '1';
        busy <= '1';
      
      when shift_wait =>
        bit_counter <= bit_counter + 1;
        byte_counter_follower <= byte_counter;
        load_data <= '0';
      
      when others => 
    
    end case;
  end if;
end process;


-- Map the RJ45 signals to the output ports
serial_data_out <= shift_data_out(shift_width-1);

-- Map the busy signal 
busy_out <= busy;

debug_out(0) <= load_data;

end architecture rtl;























































