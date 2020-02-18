----------------------------------------------------------------------------
--! @file Speaker_Array_Decoder.vhd
--! @brief Speaker array decoder component
--! @details  
--! @author Tyler Davis
--! @date 2020
--! @copyright Copyright 2020 Flat Earth inc
--
--  Permission is hereby granted, free of charge, to any person obtaining a copy
--  of this software and associated documentation files (the "Software"), to deal
--  in the Software without restriction, including without limitation the rights
--  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
--  copies of the Software, and to permit persons to whom the Software is furnished
--  to do so, subject to the following conditions:
--
--  The above copyright notice and this permission notice shall be included in all
--  copies or substantial portions of the Software.
--
--  THE SOFTWARE IS PROVIDED "AS IS", WITHout WARRANTY OF ANY KinD, EXPRESS OR IMPLIED,
--  inCLUDinG BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
--  PARTICULAR PURPOSE AND NONinFRinGEMENT. in NO EVENT SHALL THE AUTHORS OR COPYRIGHT
--  HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER in AN ACTION
--  OF CONTRACT, TORT OR OTHERWISE, ARISinG FROM, out OF OR in CONNECTION WITH THE
--  SOFTWARE OR THE USE OR OTHER DEALinGS in THE SOFTWARE.
--
-- Tyler Davis
-- Flat Earth inc
-- 985 Technology Blvd
-- Bozeman, MT 59718
-- support@flatearthinc.com
----------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

entity Speaker_Array_Decoder is
  generic (
      max_drivers : integer  := 32
    );
    port (
        sys_clk             : in  std_logic                     := '0';
        mclk                : in  std_logic                     := '0';
        reset_n             : in  std_logic                     := '0';
                
        serial_data         : in  std_logic;
        serial_control      : in  std_logic;
        serial_clk          : in  std_logic;
        
        bclk_out            : out std_logic;
        lrclk_out           : out std_logic;
        sdata_out           : out std_logic_vector(max_drivers - 1 downto 0)
    );
    
end entity Speaker_Array_Decoder;

architecture rtl of Speaker_Array_Decoder is
--------------------------------------------------------------
-- intel/Altera component to convert serial data to parallel
--------------------------------------------------------------
component Serial2Parallel_32bits
  PORT
  (
    clock           : in std_logic ;
    shiftin         : in std_logic ;
    q               : out std_logic_vector (31 downto 0)
  );
end component;

--------------------------------------------------------------
-- Altera DPR
--------------------------------------------------------------
component M9K_Buffer IS
	port
	(
		data		: in std_logic_vector (31 downto 0);
		rdaddress		: in std_logic_vector (6 downto 0);
		rdclock		: in std_logic ;
		rden		: in std_logic  := '1';
		wraddress		: in std_logic_vector (6 downto 0);
		wrclock		: in std_logic  := '1';
		wren		: in std_logic  := '0';
		q		: out std_logic_vector (31 downto 0)
	);
end component;

component FE_I2S_M9K
  generic (
    bclk_div    : unsigned(7 downto 0)  := "00000010";
    lrclk_div   : unsigned(7 downto 0)  := "00100000";
    n_drivers   : unsigned(7 downto 0)  := "00100000";
    max_drivers : integer  := 32;
    read_ahead  : integer  := 20
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
end component;

-- I2S signals
signal data_channel_r   : std_logic_vector(6 downto 0) := (others => '0');
signal valid_r          : std_logic := '0';
signal bclk_r           : std_logic := '0'; 
signal lrclk_r          : std_logic := '0';
signal sdata_out_r      : std_logic_vector(max_drivers - 1 downto 0) := (others => '0');

-- TODO tie this signal with the M-Map interface component
signal HEADER_ID      : std_logic_vector(15 downto 0) := "1100100111111010"; -- Hard coded to match encoder
signal n_drivers      : unsigned(7 downto 0)          := (others => '0');

-- Deserializer control signals
signal load_counter     : unsigned(7 downto 0) := (others => '0');
signal load_finished    : std_logic := '0';
signal state_hold       : std_logic := '0';
signal valid_data       : std_logic := '0';
signal header_found     : std_logic := '0';

-- DPR Signals
signal wren                   : std_logic := '0';
signal rden                   : std_logic := '0';
signal rden_follower          : std_logic := '0';
signal read_address           : std_logic_vector(6 downto 0)  := (others => '0');
signal read_address_follower  : std_logic_vector(6 downto 0)  := (others => '0');
signal write_address          : std_logic_vector(6 downto 0)  := (others => '0');
signal input_data_r           : std_logic_vector(31 downto 0) := (others => '0');
signal output_data_r          : std_logic_vector(31 downto 0) := (others => '0');
signal output_data_follower_r : std_logic_vector(31 downto 0) := (others => '0');
signal data_in                : std_logic_vector(31 downto 0) := (others => '0'); 


-- Shifter signals
signal parallel_data_r : std_logic_vector(31 downto 0) := (others => '0');

-- Shifter state machine signals
signal header_recieved        : std_logic := '0';
signal final_packet           : std_logic := '0';
signal bit_counter            : unsigned(4 downto 0) := (others => '0');
signal packet_counter         : unsigned(7 downto 0) := (others => '0');
signal read_counter           : unsigned(7 downto 0) := (others => '0');
signal transfer_delay         : unsigned(7 downto 0) := "00100000";
signal transfer_delay_cntr    : unsigned(7 downto 0) := (others => '0');
signal transfer_data          : std_logic := '0';
signal read_packets           : std_logic := '0';
signal last_packet            : std_logic := '0';
signal data_ready             : std_logic := '0';
signal read_all               : std_logic := '0';
signal transfer_hold          : std_logic := '0';
signal lrclk_follower_r       : std_logic := '0';

-- Create states for the output state machine
type deser_state_type is (  read_header,
                            read_packet   ); 

signal deser_state : deser_state_type;

-- Create states for the output state machine
type state_type is (idle,read_data,increment_read_address,read_finish); 
signal output_state : state_type;

begin 

-- S2P_ADC2 : Serial2Parallel_32bits PORT MAP (
  -- clock           => serial_clk,
  -- shiftin         => serial_data,
  -- q               => parallel_data_r
-- );

decoder_buffer: M9K_Buffer 
	port map
	(
		data		    => input_data_r,
		rdaddress		=> read_address,
		rdclock		  => sys_clk,
		rden		    => rden,
		wraddress		=> write_address,
		wrclock		  => serial_clk,
		wren		    => valid_r,
		q		        => output_data_r
	);

i2s_component : FE_I2S_M9K
port map (
    mclk_in             => mclk,
    sys_clk             => sys_clk,
    reset_n             => reset_n,
    
    data_input_channel  => read_address_follower,
    data_input_data     => output_data_r,
    data_input_error    => "00",
    data_input_valid    => rden_follower,
            
    bclk_out            => bclk_r,
    lrclk_out           => lrclk_r,
    sdata_out           => sdata_out_r
  );


-- I2S data loading process
i2s_load_ctrl_process : process(serial_clk,reset_n)
begin 
  if reset_n = '0' then 
    deser_state <= read_header;
  elsif rising_edge(serial_clk) then 
    case deser_state is 
      when read_header =>
        if read_packets = '1' and data_ready = '1' then 
          deser_state <= read_packet;
        else
          deser_state <= read_header;
        end if;
        
      when read_packet =>
        if last_packet = '1' then 
          deser_state <= read_header;
        else
          deser_state <= read_packet;
        end if;
        
      when others =>
      
      end case;
  end if;
end process;

shift_process: process(serial_clk,reset_n)
begin
  if reset_n = '0' then 
    parallel_data_r <= (others => '0');
  elsif rising_edge(serial_clk) then 
      parallel_data_r <= parallel_data_r(30 downto 0) & serial_data;
  end if;
end process;

i2s_load_data_process : process(serial_clk,reset_n)
begin 
  if reset_n = '0' then 
    last_packet <= '0';
  elsif rising_edge(serial_clk) then 
    case deser_state is 
      when read_header =>
        last_packet <= '0';
        data_ready    <= '1';
        if parallel_data_r(31 downto 16) = HEADER_ID(15 downto 0) then 
          read_packets <= '1';
          n_drivers <= unsigned(parallel_data_r(7 downto 0));
        else
          read_packets <= '0';
        end if;
        
      when read_packet => 
        data_ready    <= '0';
        if packet_counter = n_drivers then 
          last_packet <= '1';
          header_found <= '0';
        else
          last_packet <= last_packet;
          header_found <= '1';
        end if;
      
      when others =>
    end case;
  end if;
end process;

deser_data_process : process(serial_clk, reset_n)
begin 
  if reset_n = '0' then 
    load_counter <= (others => '0');
  elsif rising_edge(serial_clk) then 
    if header_found = '0' then 
      -- The state transitions occurs after several bits have already been transferred (Two in this case)
      load_counter <= "00000010";--(others => '0');
      packet_counter <= (others => '0');
      valid_r <= '0';
    elsif header_found = '1' and load_counter = 31 then
      load_counter <= (others => '0');
      packet_counter <= packet_counter + 1;
      -- if parallel_data_r(31) = '1' then 
        -- input_data_r <= "1111" & parallel_data_r(27 downto 0);
      -- else
        -- input_data_r <= "0000" & parallel_data_r(27 downto 0);
      -- end if;
      input_data_r <= parallel_data_r;
      write_address <= std_logic_vector(packet_counter(6 downto 0));
      valid_r <= '1';
    -- elsif header_found = '1' and load_counter = 31 then
      -- load_counter <= (others => '0');
      -- valid_r <= '0';
    elsif header_found = '1' and load_counter < 31 then 
      load_counter <= load_counter + 1;
      valid_r <= '0';
    else
      load_counter <= load_counter;
    end if;
  end if;
end process;

-- Process to control the states of the data shifting process
data_transfer_control_process : process(sys_clk,reset_n)
begin
  if reset_n = '0' then 
    output_state <= idle;
  elsif rising_edge(sys_clk) then 
  
    case output_state is 
    
      -- When idle, wait for the reading data signal to go high and make sure
      -- the data hasn't just been read out
      when idle =>
        if transfer_data = '1' and read_all = '0' then 
          output_state <= read_data;
        else
          output_state <= idle;
        end if;
     
      when read_data =>
        output_state <= increment_read_address;
      
      when increment_read_address =>
      
        -- For compatibility, the left and right channels are the even and odd channels, respectively,
        -- so when either the last even or the last odd channel is read then move to the read finish state
        if read_counter = n_drivers - 1 then 
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

data_transfer_process : process(sys_clk,reset_n)
begin
  if reset_n = '0' then 
    read_address   <= (others => '0');
    read_counter <= (others => '0');
    rden <= '0';
  elsif rising_edge(sys_clk) then 
    case output_state is 
    
      -- When idle, reset the counters and the read signal
      when idle =>
        read_address    <= (others => '0');
        read_counter    <= (others => '0');
        read_all        <= '0';
      
       when increment_read_address => 
        -- Increment the channel read counter by two and the driver counter by one
        read_counter    <= read_counter + 1;
        rden            <= '0';

      when read_data =>
        -- Enable the read
        rden          <= '1';
        read_address  <= std_logic_vector(read_counter(6 downto 0));
        
      when read_finish =>
        -- Indicate all the data has been read out
        read_all  <= '1';
        rden      <= '0';
     
      when others => 
    
    end case;
  end if;
end process;

-- Process to trigger the transfer from the decoder DPR to the I2S component
-- Note: Assumes the system clock will be faster than 32 serial clock cycles
transfer_trigger_process: process(sys_clk,reset_n)
begin 
  if reset_n = '0' then 
    transfer_data <= '0';
    transfer_hold <= '0';
  elsif rising_edge(sys_clk) then 
    
    if lrclk_follower_r /= lrclk_r then 
      transfer_delay_cntr <= (others => '0');
      transfer_hold <= '0';
    else
      transfer_delay_cntr <= transfer_delay_cntr + 1;
    end if;
    
    if transfer_delay_cntr = transfer_delay and transfer_hold = '0' then 
      transfer_data <= '1';
      transfer_hold <= '1';
    else
      transfer_data <= '0';
    end if;
    -- if transfer_hold = '0' and data_ready = '1' then 
      -- transfer_hold <= '1';
      -- transfer_data <= '1';
    -- elsif transfer_hold = '1' then 
      -- if data_ready = '0' then 
        -- transfer_hold <= '0';
      -- else
        -- transfer_hold <= '1';
      -- end if;
      -- transfer_data <= '0';
    -- else
      -- transfer_data <= '0';
    -- end if;
  end if;
end process;

follower_process: process (sys_clk)
begin 
  if rising_edge(sys_clk) then 
    rden_follower           <= rden;
    read_address_follower   <= read_address;
    output_data_follower_r  <= output_data_r;
    lrclk_follower_r        <= lrclk_r;
  end if;
end process;

bclk_out <= bclk_r;
lrclk_out <= lrclk_r;
sdata_out <= sdata_out_r;

end architecture rtl;























































