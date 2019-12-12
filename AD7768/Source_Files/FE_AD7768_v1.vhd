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
	port (
		sys_clk               : in  std_logic;                        --      clk_in.clk_in
		sys_reset_n           : in  std_logic;                        --      reset.reset_n

    spi_clk               : in  std_logic;
    double_spi_clk_in     : in  std_logic;

    -----------------------------------------------------------------------------------------------------------
    -- Abstracted data channels, i.e. interface to the data plane as 20-bit data words.
    -----------------------------------------------------------------------------------------------------------
    AD7768_data_out        : out  std_logic_vector(31 downto 0);     --      data in
    AD7768_valid_out       : out  std_logic;
    AD7768_error_out       : out  std_logic_vector(1 downto 0);
    
    -------------------------------------------------------------------------------------------------------------------------------------
    -- AD7768 Physical Layer : Signals to/from AD7768 Serial Data Port (from ADCs/to DACs), i.e. connection to physical pins on AD7768
    -------------------------------------------------------------------------------------------------------------------------------------
    AD7768_MISO_out       : in  std_logic;                        --      MISO for the SPI line
    AD7768_MOSI_in        : out std_logic;                        --      MISO for the SPI line
    AD7768_SCLK_out       : out std_logic;
    AD7768_CONV_out       : out std_logic
	);
end entity FE_AD7768_v1;

architecture rtl of FE_AD7768_v1 is


  component spi_commands is
	  generic(
	
	  command_used_g          : std_logic 	:= '1';
	  address_used_g          : std_logic 	:= '1';
	  command_width_bytes_g   : natural 	:= 1;
	  address_width_bytes_g   : natural 	:= 1;
	  data_length_bit_width_g : natural 	:= 8;
	  cpol_cpha               : std_logic_vector(1 downto 0) := "00"
	  );
		port(
			clk	           :in	std_logic;	
			rst_n 	        :in	std_logic;
			
			command_in      : in  std_logic_vector(command_width_bytes_g*8-1 downto 0);
			address_in      : in  std_logic_vector(address_width_bytes_g*8-1 downto 0);
			address_en_in   : in  std_logic;
			data_length_in  : in  std_logic_vector(data_length_bit_width_g - 1 downto 0);
			
			master_slave_data_in      :in   std_logic_vector(7 downto 0);
			master_slave_data_rdy_in  :in   std_logic;
			master_slave_data_ack_out :out  std_logic;
			command_busy_out          :out  std_logic;
			command_done              :out  std_logic;
	
			slave_master_data_out     : out std_logic_vector(7 downto 0);
			slave_master_data_ack_out : out std_logic;
	
			miso 				:in	std_logic;	
			mosi 				:out  std_logic;	
			sclk 				:out  std_logic;	
			cs_n 				:out  std_logic 
		);
	end component;
  
  component spi_clk_delay is
    port (
      spi_clk                   : in  std_logic;
      double_spi_clk            : in  std_logic;
      sys_reset                 : in  std_logic;
      sclk_out                  : out  std_logic
      );
  end component;
  
  signal AD7768_data_r                  : std_logic_vector(31 downto 0);
  
  -----------------------------------------------------------------------------------------------------
	-- SPI related signals
  -----------------------------------------------------------------------------------------------------
  signal   AD7768_spi_command       	  : std_logic_vector(7 downto 0);  
  signal   AD7768_spi_register_address  : std_logic_vector(7 downto 0);  
  signal   AD7768_spi_write_data        : std_logic_vector(7 downto 0);   -- data to be written to AD7768 register
  signal   AD7768_spi_write_data_rdy    : std_logic;                      -- assert (clock pulse) to write data
  signal   AD7768_spi_busy          	  : std_logic;                      -- If 1, the spi is busy servicing a command. Wait until 0 to send another command. 
  signal   AD7768_spi_done          	  : std_logic;                      
  signal   AD7768_spi_read_data         : std_logic_vector(7 downto 0);   -- data read from AD7768 register
  signal   AD7768_spi_read_data_ack  	  : std_logic;                      -- data ready to be read
  signal   AD7768_spi_sclk              : std_logic;
  signal   AD7768_spi_sclk_delayed      : std_logic;
  signal   AD7768_conv                  : std_logic;
  signal   AD7768_valid                 : std_logic := '0';
  signal   AD7768_data                  : std_logic_vector(19 downto 0) := "00000000000000000000";
  
  -- Data "addresses"
  signal control_reg                    : std_logic_vector(5 downto 0) := "010100";
  
  -- Data read and write commands
  signal write_data                     : std_logic := '0';
  signal read_data                      : std_logic := '1';
  signal data_ready                     : std_logic := '0';
    
  -- Command register bits defined on pg. 25 of the manual
  signal WEN                            : std_logic := '0';
  signal RESERVED                       : std_logic := '0'; -- Reserved bits should be programmed as zeros
  signal TURBO                          : std_logic := '1'; -- Default to disabled
  signal OV                             : std_logic := '0'; -- Default to disabled
  signal HIGHZ                          : std_logic := '0'; -- Default to disabled
  signal SPAN_COMPRESSION               : std_logic := '0'; -- Default to disabled
  signal ENABLE_STATUS_BITS             : std_logic := '0'; -- Default to disabled
  
  signal RDOP                           : std_logic_vector(7 downto 0) := "00000000";
  
  signal reg_config                     : std_logic_vector(7 downto 0)  := RESERVED & RESERVED & RESERVED & ENABLE_STATUS_BITS &
                                                                           SPAN_COMPRESSION & HIGHZ & TURBO & OV;
  -- ADC init flag
  signal init_done                      : std_logic := '0'; 
  
  type state_type is {
    init_adc,
    spi_init_load
    spi_data_load,
    spi_read_data_busy,
    spi_read_data_start,
    spi_write_init_busy,
    spi_write_init_start,
    spi_write_init_start
  };
  
  signal state : state_type;
  
begin

  -----------------------------------------------------------------------
  -- SPI interface to the AD7768 SPI Control Port
  -----------------------------------------------------------------------
  spi_AD7768: spi_commands
  generic map (
    command_used_g            => '1',  -- command field is used
    address_used_g            => '1',  -- address field is used
    command_width_bytes_g     =>  1,   -- command is 1 byte
    address_width_bytes_g     =>  1,   -- address is 1 byte
    data_length_bit_width_g   =>  8,   -- data length is 8 bits
    cpol_cpha                 => "00"  -- AD7768:  CPOL=0, CPHA=0  This is actually CPOL=0, CPHA=1 as implemented
  )
  port map (
    clk	                      => spi_clk    ,  					-- spi clock (50 MHz max)
    rst_n 	                  => sys_reset_n,		   				    -- component reset
    command_in                => AD7768_spi_command,  				-- Command includes Global Address (0000100) and is either Read ("00001001") or Write ("00001000").
    address_in                => AD7768_spi_register_address,  	-- Register Address.  
    address_en_in             => '1',          						-- 1=Address field will be used.
    data_length_in            => "00000001",   						-- Data payload will be 1 byte ("00000001").	
    master_slave_data_in      => AD7768_spi_write_data,			-- data to be written to an AD7768 register
    master_slave_data_rdy_in  => AD7768_spi_write_data_rdy,    	-- assert (clock pulse) to write the data
    master_slave_data_ack_out => open,                         	-- ignore acknowledgement 
    command_busy_out          => AD7768_spi_busy,					-- If 1, the spi is busy servicing a command. 
    command_done              => AD7768_spi_done,					-- pulse signals end of command
    slave_master_data_out     => AD7768_spi_read_data,				-- data read from AD7768 register
    slave_master_data_ack_out => AD7768_spi_read_data_ack,		-- data ready to be read
    miso 				              => AD7768_MISO_out,					-- AD7768 SPI signal = data from AD7768 SPI registers
    mosi 					            => AD7768_MOSI_in,						-- AD7768 SPI signal = data to AD7768 SPI registers
    sclk 					            => AD7768_spi_sclk,					-- AD7768 SPI signal = sclk: serial clock
    cs_n 					            => open				-- AD7768 SPI signal = ss_n: slave select (active low)
  );
  
  spi_delay: spi_clk_delay
    port map (
      spi_clk                   => AD7768_spi_sclk,
      double_spi_clk            => double_spi_clk_in,
      sys_reset                 => sys_reset_n,
      sclk_out                  => AD7768_spi_sclk_delayed
      );
      
  -- State machine that waits for a valid pulse from the FPGA, assumes the 
  -- system clock is faster or equal to the SPI clock
  process(sys_clk,sys_reset_n)
  begin
    if rising_edge(sys_clk) then 
      if ((AD7768_valid_in = '1' and AD7768_spi_busy = '0' and data_ready = '0') 
        or (data_ready = '1' and AD7768_spi_busy = '0')) then 
        data_ready <= '1';
      else 
        data_ready <= '0';
      end if;
    end if;
  end process;
  
  -- State machine that handles the state transitions 
  process (spi_clk, sys_reset_n)
  begin
  
    -- Reset the signals when the system reset is deasserted
    if sys_reset_n = '0' then
      state     <= init_ADC;  
      init_done <= '0';
      
    -- If the reset is not asserted, 
    elsif (rising_edge(spi_clk)) then
      case state is  
        when init_ADC =>
          if init_done = '0' then 
            state <= spi_init_load;
            init_done <= '1';
          elsif init_done = '1' then 
            state <= spi_data_wait;
          else
            state <= init_ADC;
          end if;
          
        when spi_init_load => 
          state <= spi_write_init_start;
          
        when spi_write_init_start =>
          if AD7768_spi_busy = '1' then 
            state <= spi_write_init_busy;
          else
            state <= spi_write_init_start;
          end if;
          
        when spi_write_init_busy =>
          if AD7768_spi_busy = '1' then 
            state <= spi_write_init_busy;
          else
            state <= spi_data_load;
          end if; 
          
        when spi_data_load => 
          state <= spi_read_data_start;

        when spi_read_data_start =>
          if AD7768_spi_busy = '1' then 
            state <= spi_read_data_busy;
          else
            state <= spi_read_data_start;
          end if;
          
        when spi_read_data_busy =>
        if AD7768_spi_busy = '1' then 
          state <= spi_read_data_busy;
        else
          state <= spi_read_data_finish;
        end if;   

        when spi_read_data_finish =>
          state <= spi_read_data_start;
          
        when others =>
          state <= init_ADC;
          
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
      case state is     
      
        when init_ADC =>
      
        when spi_init_load =>
          -- Workaround for transferring 24 data bits 
          AD7768_spi_command              <= write_data & control_reg & reg_config_reserved(19 downto 16);
          AD7768_spi_register_address     <= reg_config_reserved(15 downto 8);
          AD7768_spi_write_data 			    <= reg_config_reserved(7 downto 0);
          
        when spi_write_init_start =>
          AD7768_spi_write_data <= '1';
          
        when spi_write_init_busy =>
          AD7768_spi_write_data <= '0';
      
        when spi_data_load =>
          AD7768_conv <= '1';
          
          -- Workaround for transferring 24 data bits 
          AD7768_spi_command              <= write_data & control_reg & reg_config_reserved(19 downto 16);
          AD7768_spi_register_address     <= reg_config_reserved(15 downto 8);
          AD7768_spi_write_data 			    <= reg_config_reserved(7 downto 0);
          
        when spi_read_data_start =>
          AD7768_conv <= '0';
          AD7768_valid <= '0';
          
        when spi_read_data_busy =>
          AD7768_spi_write_data <= '0';
          
        when spi_read_data_finish =>
          AD7768_valid <= '1';
          AD7768_data <= AD7768_spi_read_data;
        
        when others => 
        -- do nothing
      end case;
    end if;
  end process;
  

  -- Map the output signals
  AD7768_SCLK_out <= AD7768_spi_sclk_delayed;
  AD7768_CONV_out <= AD7768_conv;
  AD7768_data_out <= AD7768_data;
  
  

end architecture rtl; -- of FE_AD7768_v1






































