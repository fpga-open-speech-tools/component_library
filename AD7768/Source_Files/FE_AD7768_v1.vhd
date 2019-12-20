----------------------------------------------------------------------------
--! @file FE_AD7768_v1.vhd
--! @brief Implements serial/streaming data transfer for the AD7768 ADC.
--! @details This 
--! @author Tyler Davis
--! @date 2019
--! @copyright Copyright 2019 Flat Earth Inc
--
-- THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
-- INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR 
-- PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE 
-- FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, 
-- ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
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

entity FE_AD7768_v1 is
  generic (
    n_channels   : integer := 8
  );
	port (
		sys_clk               : in  std_logic;
		sys_reset_n           : in  std_logic;
    spi_clk               : in  std_logic;

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
    AD7768_DRDY_in        : in std_logic;
    AD7768_MISO_out       : in  std_logic;
    AD7768_MOSI_in        : out std_logic;
    AD7768_SCLK_out       : out std_logic;
    AD7768_CSN_out        : out std_logic
	);
end entity FE_AD7768_v1;

architecture rtl of FE_AD7768_v1 is


  -- Declare the SPI component
  component spi_commands is
	  generic(
	
	  command_used_g          : std_logic 	:= '1';
	  address_used_g          : std_logic 	:= '1';
	  command_width_bits_g   : natural 	:= 8;
	  address_width_bits_g   : natural 	:= 8;
    data_width_bits_g 	  : natural 	:= 8;
    output_bits_g            : natural   := 24;
	  cpol_cpha               : std_logic_vector(1 downto 0) := "10"
	  );
		port(
			clk	           :in	std_logic;	
			rst_n 	        :in	std_logic;
			
			command_in      : in  std_logic_vector(command_width_bits_g-1 downto 0);
			address_in      : in  std_logic_vector(address_width_bits_g-1 downto 0);
			
			master_slave_data_in      :in   std_logic_vector(data_width_bits_g-1 downto 0);
			master_slave_data_rdy_in  :in   std_logic;
			master_slave_data_ack_out :out  std_logic;
			command_busy_out          :out  std_logic;
			command_done              :out  std_logic;
	
			slave_master_data_out     : out std_logic_vector(output_bits_g-1 downto 0);
			slave_master_data_ack_out : out std_logic;
	
			miso 				:in	std_logic;	
			mosi 				:out  std_logic;	
			sclk 				:out  std_logic;	
			cs_n 				:out  std_logic 
		);
	end component;
  
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
  signal s2pout_array                   : data_array  := (others => (others => '0'));
  signal deser_array_r                  : data_array  := (others => (others => '0'));
  signal deser_array_sreg               : data_sreg   := (others => (others => '0'));
  
  signal channel_counter                : integer range 0 to 8 := 0;
  signal bit_counter                    : integer range 0 to 32 := 0;
  signal setting_counter                : integer range 0 to 4 := 0;
  signal n_bits                         : integer := 32;
  signal n_settings                     : integer := 4;
  
  type spi_args is array (n_settings - 1 downto 0) of std_logic_vector(7 downto 0);
  
  -- Settings and addresses found in the AD7768 datasheet pg. 78 - 104
  --                                    Chan en    Widebandx32  MCLK_DIV    DCLK_DIV
  signal spi_addr       : spi_args := ("00000000", "00000001", "00000100", "00000111");
  signal spi_cmds       : spi_args := ("11111100", "00000000", "00110011", "00000000");
  

  -----------------------------------------------------------------------------------------------------
	-- SPI related signals
  -----------------------------------------------------------------------------------------------------
  signal   AD7768_spi_command       	  : std_logic_vector(7 downto 0);  
  signal   AD7768_spi_register_address  : std_logic_vector(7 downto 0);  
  signal   AD7768_spi_write_data        : std_logic_vector(7 downto 0);   -- data to be written to AD7768 register
  signal   AD7768_spi_write_data_rdy    : std_logic;                      -- assert (clock pulse) to write data
  signal   AD7768_spi_busy          	  : std_logic;                      -- If 1, the spi is busy servicing a command. Wait until 0 to send another command. 
  signal   AD7768_spi_done          	  : std_logic;                      
  signal   AD7768_spi_read_data         : std_logic_vector(15 downto 0);   -- data read from AD7768 register
  signal   AD7768_spi_read_data_ack  	  : std_logic;                      -- data ready to be read
  signal   AD7768_spi_sclk              : std_logic;
  signal   AD7768_spi_sclk_delayed      : std_logic;
  signal   AD7768_conv                  : std_logic;
  signal   AD7768_valid_r                 : std_logic := '0';
  signal   AD7768_channel_out_r         : std_logic_vector(2 downto 0) := (others => '0');
  
  -- Data "addresses"
  signal control_reg                    : std_logic_vector(5 downto 0) := "010100";
  
  -- Data read and write commands
  signal write_data                     : std_logic := '0';
  signal read_data                      : std_logic := '1';
  signal data_ready                     : std_logic := '0';
  signal data_valid                     : std_logic := '0';
  signal data_reg_start                 : std_logic := '0';
    
  -- Command register bits defined on pg. 25 of the manual
  signal WEN                            : std_logic := '0';
  signal RESERVED                       : std_logic := '0'; -- Reserved bits should be programmed as zeros
  signal CH_EN                          : std_logic := '0'; -- Enable the channel
  signal CH_DIS                         : std_logic := '1'; -- Disable the channel
  
  signal RDOP                           : std_logic_vector(7 downto 0) := "00000000";
  
  -- ADC init flag
  signal init_done                      : std_logic := '0'; 
  
  type spi_state_type is 
  (
    init_adc,
    spi_init_load,
    spi_init_done,
    spi_increment_command,
    spi_write_init_busy,
    spi_write_init_start
  );
  
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
  
  signal spi_state : spi_state_type;
  signal data_state : data_state_type;
begin


  -- -- Assign the SPI commands
  -- spi_cmds(0)   <= CH_DIS & CH_DIS & CH_DIS & CH_DIS &
                   -- CH_DIS & CH_DIS & CH_EN & CH_EN;
  -- spi_addr(0)   <= (others => '0');   
  
  -- -- Set the SINC filter to x64
  -- spi_cmds(1)   <= "00001001";
  -- spi_addr(1)   <= "00000001"; 

  -- -- Set the MCLK divider and the power mode
  -- spi_cmds(2)   <= "00110011";
  -- spi_addr(2)   <= "00000100"; 
  
  -- -- Set the DCLK divider
  -- spi_cmds(3)   <= "00000001";
  -- spi_addr(3)   <= "00000111"; 
  
  -----------------------------------------------------------------------
  -- SPI interface to the AD7768 SPI Control Port
  -----------------------------------------------------------------------
  spi_AD7768: spi_commands
  generic map (
    command_used_g            => '0',                           -- command field is used
    address_used_g            => '1',                           -- address field is used
    command_width_bits_g      =>  8,                            -- command is 1 byte
    address_width_bits_g      =>  8,                            -- address is 1 byte
    data_width_bits_g 	      =>  8,
    output_bits_g             =>  16,
    cpol_cpha                 => "10"  
  )
  port map (
    clk	                      => spi_clk    ,  					        -- spi clock (50 MHz max)
    rst_n 	                  => sys_reset_n,		   				      -- component reset
    
    command_in                => AD7768_spi_command,  			
    address_in                => AD7768_spi_register_address,  	-- Register Address.  
    
    master_slave_data_in      => AD7768_spi_write_data,			    -- data to be written to an AD7768 register
    master_slave_data_rdy_in  => AD7768_spi_write_data_rdy,    	-- assert (clock pulse) to write the data
    master_slave_data_ack_out => open,                         	-- ignore acknowledgement 
    command_busy_out          => AD7768_spi_busy,					      -- If 1, the spi is busy servicing a command. 
    command_done              => AD7768_spi_done,					      -- pulse signals end of command
    
    slave_master_data_out     => AD7768_spi_read_data,				  -- data read from AD7768 register
    slave_master_data_ack_out => AD7768_spi_read_data_ack,		  -- data ready to be read
    
    miso 				              => AD7768_MISO_out,					      -- AD7768 SPI signal = data from AD7768 SPI registers
    mosi 					            => AD7768_MOSI_in,						    -- AD7768 SPI signal = data to AD7768 SPI registers
    sclk 					            => AD7768_spi_sclk,					      -- AD7768 SPI signal = sclk: serial clock
    cs_n 					            => AD7768_CSN_out				          -- AD7768 SPI signal = ss_n: slave select (active low)
  );
  
  
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
    
  -- State machine that handles the spi_state transitions 
  process (spi_clk, sys_reset_n)
  begin
  
    -- Reset the signals when the system reset is deasserted
    if sys_reset_n = '0' then
      spi_state     <= init_ADC;  
      init_done     <= '0';
      
    -- If the reset is not asserted, 
    elsif (rising_edge(spi_clk)) then
      case spi_state is  
        when init_ADC =>
          if init_done = '0' then 
            spi_state <= spi_init_load;
            init_done <= '1';
          elsif init_done = '1' then 
            spi_state <= spi_init_done;
          else
            spi_state <= init_ADC;
          end if;
          
        when spi_init_load => 
          spi_state <= spi_write_init_start;
          
        when spi_write_init_start =>
          if AD7768_spi_busy = '1' then 
            spi_state <= spi_write_init_busy;
          else
            spi_state <= spi_write_init_start;
          end if;
          
        when spi_write_init_busy =>
          if AD7768_spi_busy = '1' then 
            spi_state <= spi_write_init_busy;
          else
            spi_state <= spi_increment_command;
          end if; 
          
        when spi_increment_command =>
          if setting_counter = n_settings - 1 then 
            spi_state <= spi_init_done;
          else
            spi_state <= spi_init_load;
          end if;
          
        when spi_init_done => 
        spi_state <= spi_init_done;
          
        when others =>
          spi_state <= init_ADC;
          
      end case;
        
    end if;
  end process;

    --------------------------------------------------------------
    -- State Machine to implement Avalon streaming
    -- Generate Avalon streaming signals
    --------------------------------------------------------------
  process (spi_clk)
  begin
    if (rising_edge(spi_clk)) then
      case spi_state is     
      
        when init_ADC =>
      
        when spi_init_load =>
          -- Workaround for transferring 24 data bits 
          -- AD7768_spi_command              <= write_data & control_reg & reg_config(19 downto 16);
          AD7768_spi_register_address     <= spi_addr(setting_counter);
          AD7768_spi_write_data 			    <= spi_cmds(setting_counter);
          
        when spi_write_init_start =>
          AD7768_spi_write_data_rdy <= '1';
          
        when spi_write_init_busy =>
          AD7768_spi_write_data_rdy <= '0';
          
        when spi_increment_command =>
          setting_counter <= setting_counter + 1;
        
        when spi_init_done =>
        
        when others => 
        -- do nothing
      end case;
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
            conv_data_r  <= "1111" & conv_data_r(23 downto 0) & "0000";
          else
            conv_data_r  <= "0000" & conv_data_r(23 downto 0) & "0000";
          end if;
          
          -- The commented code below has a clear tone, but tons of white noise
          -- if conv_data_r(23) = '1' then 
            -- conv_data_r  <= "111" & conv_data_r(23 downto 0) & "00000";
          -- else
            -- conv_data_r  <= "000" & conv_data_r(23 downto 0) & "00000";
          -- end if;

        when read_data_valid =>
          -- AD7768_data_r         <= std_logic_vector(resize(signed(std_logic_vector(deser_array_r(channel_counter)(23 downto 0)) & "0000"),32));
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






































