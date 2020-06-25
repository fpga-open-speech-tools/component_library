----------------------------------------------------------------------------
--! @file FE_AD7768_v1.vhd
--! @brief Implements serial/streaming data transfer for the AD7768 ADC.
--! @details This 
--! @author Tyler Davis
--! @date 2020
--! @copyright Copyright 2020 Audio Logic
--
-- THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
-- INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR 
-- PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE 
-- FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, 
-- ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
--
-- Tyler Davis
-- Audio Logic
-- 985 Technology Blvd
-- Bozeman, MT 59718
-- openspeech@flatearthinc.com
----------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

entity FE_AD7768_v1 is
  generic (
    n_channels   : integer := 4
  );
	port (
		sys_clk               : in  std_logic;
		sys_reset_n           : in  std_logic;

    -----------------------------------------------------------------------------------------------------------
    -- Avalon Streaming Interface
    -----------------------------------------------------------------------------------------------------------
    AD7768_data_out        : out  std_logic_vector(31 downto 0);
    AD7768_valid_out       : out  std_logic;
    AD7768_error_out       : out  std_logic_vector(1 downto 0);
    AD7768_channel_out     : out  std_logic_vector(2 downto 0);
    
    -----------------------------------------------------------------------------------------------------------
    -- AD7768 Physical Layer
    -----------------------------------------------------------------------------------------------------------
    AD7768_DOUT_in        : in std_logic_vector(n_channels - 1 downto 0);
    AD7768_DCLK_in        : in  std_logic;
    AD7768_DRDY_in        : in std_logic
	);
end entity FE_AD7768_v1;

architecture rtl of FE_AD7768_v1 is

  --------------------------------------------------------------
  -- Intel/Altera component to convert serial data to parallel
  --------------------------------------------------------------
  component Serial2Parallel_32bits
    port
    (
      clock           : in std_logic ;
      shiftin         : in std_logic ;
      q               : out std_logic_vector (31 downto 0)
    );
  end component;
  
  signal AD7768_data_r                  : std_logic_vector(31 downto 0);
  signal conv_data_r                    : std_logic_vector(31 downto 0);
      
  type data_array is array (n_channels - 1 downto 0) of std_logic_vector(31 downto 0);
  type data_sreg  is array (n_channels - 1 downto 0) of std_logic_vector(24 downto 0);
  
  signal deser_array                    : data_array  := (others => (others => '0'));
  signal deser_array_r                  : data_array  := (others => (others => '0'));
  
  signal channel_counter                : integer range 0 to 8 := 0;
  signal bit_counter                    : integer range 0 to 32 := 0;
  signal n_bits                         : integer := 32;
     
  signal   AD7768_valid_r                 : std_logic := '0';
  signal   AD7768_channel_out_r         : std_logic_vector(2 downto 0) := (others => '0');
   
  -- Data read and write commands
  signal data_valid                     : std_logic := '0';
  signal data_reg_start                 : std_logic := '0';
      
  type data_state_type is 
  (
    data_read,
    data_wait,
    data_hold,
    read_data_register,
    read_data_convert,
    increment_channel,
    read_data_capture,
    read_data_valid
  );
  
  signal data_state : data_state_type;
begin  
  
  load_shift_generate: for ch_ind in n_channels - 1 downto 0 generate
  serial_shift_map: Serial2Parallel_32bits
    port map 
    (  
      clock           => AD7768_DCLK_in,
      shiftin         => AD7768_DOUT_in(ch_ind),
      q               => deser_array(ch_ind)
    );
  end generate;
  
  data_register_process: process(AD7768_DCLK_in)
  begin 
    if rising_edge(AD7768_DCLK_in) then 
      if AD7768_DRDY_in = '1' then 
        data_reg_start <= '1';
      elsif bit_counter = n_bits - 1 and data_reg_start = '1' then 
        data_valid <= '1';
        bit_counter <= 0;
        data_reg_start <= '0';
      elsif data_reg_start = '1' then 
        data_valid <= '0';
        bit_counter <= bit_counter + 1;
      else
        data_valid <= '0';
      end if;
    end if;
  end process;
      
  -- State machine that handles the data_state transitions 
  process (sys_clk, sys_reset_n)
  begin
  
    -- Reset the signals when the system reset is deasserted
    if sys_reset_n = '0' then
      data_state     <= data_read;  
      
    -- If the reset is not asserted, 
    elsif (rising_edge(sys_clk)) then
      case data_state is  
        when read_data_capture =>
            data_state <= read_data_register;
          
        when read_data_register =>       
            data_state <= read_data_convert;
            
        when read_data_convert =>
          data_state <= read_data_valid;
          
        when read_data_valid =>
          data_state <= increment_channel;
          
        when increment_channel =>
          if channel_counter = n_channels - 1 then 
            data_state <= read_data_register;
          else
            data_state <= data_hold;
          end if;
          
        when data_hold =>
          if data_valid = '0' then 
            data_state <= data_wait;
          else
            data_state <= data_hold;
          end if;
          
        when data_wait =>
          if data_valid = '1' then 
            data_state <= read_data_capture;
          else
            data_state <= data_wait;
          end if;
        
        when others =>
          data_state <= data_wait;
          
      end case;
        
    end if;
  end process;

    --------------------------------------------------------------
    -- State Machine to implement Avalon streaming
    -- Generate Avalon streaming signals
    --------------------------------------------------------------
  process (sys_clk)
  begin
    if (rising_edge(sys_clk)) then
      case data_state is     

        when read_data_capture =>
          deser_array_r <= deser_array;
          
        when read_data_register => 
          conv_data_r <= deser_array_r(channel_counter);
          
        when read_data_convert =>
          if conv_data_r(23) = '1' then 
            conv_data_r  <= "111" & conv_data_r(23 downto 0) & "00000";
          else
            conv_data_r  <= "000" & conv_data_r(23 downto 0) & "00000";
          end if;

        when read_data_valid =>
          AD7768_data_r         <= conv_data_r;
          AD7768_channel_out_r  <= std_logic_vector(to_unsigned(channel_counter,AD7768_channel_out'length));
          AD7768_valid_r        <= '1';

        when increment_channel =>
          AD7768_valid_r  <= '0';
          channel_counter <= channel_counter + 1;
          
        when data_hold =>
          channel_counter <= 0;
          
        when data_wait =>
        
        when others => 
        -- do nothing
      end case;
    end if;
  end process;
  
  -- Mapping for the Avalon streaming interface
  AD7768_channel_out  <= AD7768_channel_out_r;
  AD7768_data_out     <= AD7768_data_r;
  AD7768_valid_out    <= AD7768_valid_r;
  AD7768_error_out    <= (others => '0');
  
end architecture rtl; -- of FE_AD7768_v1






































