----------------------------------------------------------------------------
--! @file FE_I2S_M10K.vhd
--! @brief 
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

entity FE_I2S_M10K is
  generic (
    bclk_div : unsigned(7 downto 0) := "00000100";
    lrclk_div : unsigned(7 downto 0) := "00100000"
  );
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
    
end entity FE_I2S_M10K;

architecture rtl of FE_I2S_M10K is

component I2S_DPR is
	port
	(
		data		: in std_logic_vector (31 downto 0);
		rdaddress		: in std_logic_vector (3 downto 0);
		rdclock		: in std_logic ;
		rden		: in std_logic  := '1';
		wraddress		: in std_logic_vector (3 downto 0);
		wrclock		: in std_logic ;
		wren		: in std_logic  := '0';
		q		: out std_logic_vector (31 downto 0)
	);
end component;

signal bclk_counter  : unsigned(8 downto 0) := (others => '0');
signal lrclk_counter : unsigned(7 downto 0) := (others => '0');

signal bclk_r : std_logic := '0';
signal lrclk_r : std_logic := '0';
signal sdata_r : std_logic_vector(31 downto 0) := (others => '0');

-- DPR Signals
signal wren           : std_logic := '0';
signal rden           : std_logic := '0';
signal read_address   : std_logic_vector(3 downto 0) := (others => '0');
signal write_address  : std_logic_vector(3 downto 0) := (others => '0');
signal input_data_r   : std_logic_vector(31 downto 0) := (others => '0');
signal output_data_r  : std_logic_vector(31 downto 0) := (others => '0');
signal data_in  : std_logic_vector(31 downto 0) := (others => '0'); 

signal lrclk_div_shifted : unsigned(7 downto 0) := lrclk_div(7 downto 0);


-- Create states for the output state machine
type state_type is (idle,shift_header,shift_wait,shift_data,read_data,increment_read_address,shift_finish); 
signal output_state : state_type;

begin 
-- Map the DPR
i2s_buffer : I2S_DPR
port map (
  data => input_data_r,
  rdaddress => read_address,
  rdclock => bclk_r,
  rden => rden,
  wraddress => write_address,
  wrclock => sys_clk,
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
      write_address <= "000" & data_input_channel;
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
    if bclk_counter = bclk_div then 
      bclk_counter  <= (others => '0');
      bclk_r        <= not bclk_r;
    else
      bclk_counter  <= bclk_counter + 1;
    end if;
  end if;
end process;

lrclk_process : process(bclk_r,reset_n)
begin 
  if reset_n = '0' then 
    lrclk_r <= '0';
  elsif rising_edge(bclk_r) then 
    if bclk_counter = bclk_div and lrclk_counter = lrclk_div_shifted then 
      lrclk_r <= not lrclk_r;
      lrclk_counter <= (others => '0');
    elsif bclk_counter = bclk_div and lrclk_counter < lrclk_div_shifted then 
      lrclk_counter <= lrclk_counter + 1;
    else
      lrclk_counter <= lrclk_counter;
    end if;
  end if;
end process;

sdata_process : process(bclk_r,reset_n)
begin 
  if reset_n = '0' then 
    sdata_r <= (others => '0');
  elsif rising_edge(bclk_r) then 
    if lrclk_counter = lrclk_div_shifted - 1 then 
      sdata_r <= output_data_r;
      read_address <= "000" & not lrclk_r;
      rden <= '1';
    else
      sdata_r <= sdata_r(30 downto 0) & '0';
      rden <= '0';
    end if;
  end if;
end process;

-- Map the signals to the output ports
bclk_out  <= bclk_r;
lrclk_out <= lrclk_r;
sdata_out <= sdata_r(to_integer(lrclk_div - 1));

end architecture rtl;























































