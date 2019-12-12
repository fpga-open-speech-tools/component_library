----------------------------------------------------------------------------
--! @file FE_AD4020_v1.vhd
--! @brief Implements serial/streaming data transfer for the AD4020 ADC.
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

entity FE_AD4020_v1 is
	port (
		sys_clk               : in  std_logic;                        --      clk_in.clk_in
		sys_reset_n           : in  std_logic;                        --      reset.reset_n

    spi_clk               : in  std_logic;
    double_spi_clk_in     : in  std_logic;

    -----------------------------------------------------------------------------------------------------------
    -- Abstracted data channels, i.e. interface to the data plane as 20-bit data words.
    -----------------------------------------------------------------------------------------------------------
    AD4020_data_out        : out  std_logic_vector(31 downto 0);     --      data in
    AD4020_valid_out       : out  std_logic;
    AD4020_error_out       : out  std_logic_vector(1 downto 0);
    
    -------------------------------------------------------------------------------------------------------------------------------------
    -- AD4020 Physical Layer : Signals to/from AD4020 Serial Data Port (from ADCs/to DACs), i.e. connection to physical pins on AD4020
    -------------------------------------------------------------------------------------------------------------------------------------
    AD4020_MISO_out       : in  std_logic;                        --      MISO for the SPI line
    AD4020_MOSI_in        : out std_logic;                        --      MISO for the SPI line
    AD4020_SCLK_out       : out std_logic;
    AD4020_CONV_out       : out std_logic
	);
end entity FE_AD4020_v1;

architecture rtl of FE_AD4020_v1 is

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
  
  component spi_clk_delay is
    port (
      spi_clk                   : in  std_logic;
      double_spi_clk            : in  std_logic;
      sys_reset                 : in  std_logic;
      sclk_out                  : out  std_logic
      );
  end component;
  
  signal AD4020_data_r                  : std_logic_vector(31 downto 0);
  
  -----------------------------------------------------------------------------------------------------
	-- SPI related signals
  -----------------------------------------------------------------------------------------------------
  signal   AD4020_spi_command       	  : std_logic_vector(7 downto 0);  
  signal   AD4020_spi_register_address  : std_logic_vector(7 downto 0);  
  signal   AD4020_spi_write_data        : std_logic_vector(7 downto 0);   -- data to be written to AD4020 register
  signal   AD4020_spi_write_data_rdy    : std_logic;                      -- assert (clock pulse) to write data
  signal   AD4020_spi_busy          	  : std_logic;                      -- If 1, the spi is busy servicing a command. Wait until 0 to send another command. 
  signal   AD4020_spi_done          	  : std_logic;                      
  signal   AD4020_spi_read_data         : std_logic_vector(7 downto 0);   -- data read from AD4020 register
  signal   AD4020_spi_read_data_ack  	  : std_logic;                      -- data ready to be read
  signal   AD4020_spi_sclk              : std_logic;
  signal   AD4020_spi_sclk_delayed      : std_logic;
  signal   AD4020_conv                  : std_logic;
  signal   AD4020_valid                 : std_logic := '0';
  signal   AD4020_data                  : std_logic_vector(19 downto 0) := "00000000000000000000";
  
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
  -- SPI interface to the AD5791 SPI Control Port
  -----------------------------------------------------------------------
  spi_AD5791: spi_commands
  generic map (
    command_used_g            => '1',  -- command field is used
    address_used_g            => '1',  -- address field is used
    command_width_bits_g      =>  8,   -- command is 1 byte
    address_width_bits_g      =>  8,   -- address is 1 byte
    data_width_bits_g 	      =>  8,
    output_bits_g               =>  24,
    cpol_cpha                 => "10"  -- AD5791:  CPOL=0, CPHA=0  This is actually CPOL=0, CPHA=1 as implemented
  )
  port map (
    clk	                      => spi_clk    ,  					-- spi clock (50 MHz max)
    rst_n 	                  => sys_reset_n,		   				    -- component reset
    
    command_in                => AD5791_spi_command,  				-- Command includes Global Address (0000100) and is either Read ("00001001") or Write ("00001000").
    address_in                => AD5791_spi_register_address,  	-- Register Address.  
    
    master_slave_data_in      => AD5791_spi_write_data,			-- data to be written to an AD5791 register
    master_slave_data_rdy_in  => AD5791_spi_write_data_rdy,    	-- assert (clock pulse) to write the data
    master_slave_data_ack_out => open,                         	-- ignore acknowledgement 
    command_busy_out          => AD5791_spi_busy,					-- If 1, the spi is busy servicing a command. 
    command_done              => AD5791_spi_done,					-- pulse signals end of command
    
    slave_master_data_out     => AD5791_spi_read_data,				-- data read from AD5791 register
    slave_master_data_ack_out => AD5791_spi_read_data_ack,		-- data ready to be read
    
    miso 				              => AD5791_MISO_out,					-- AD5791 SPI signal = data from AD5791 SPI registers
    mosi 					            => AD5791_MOSI_in,						-- AD5791 SPI signal = data to AD5791 SPI registers
    sclk 					            => AD5791_spi_sclk,					-- AD5791 SPI signal = sclk: serial clock
    cs_n 					            => open				-- AD5791 SPI signal = ss_n: slave select (active low)
  );
  
  spi_delay: spi_clk_delay
    port map (
      spi_clk                   => AD4020_spi_sclk,
      double_spi_clk            => double_spi_clk_in,
      sys_reset                 => sys_reset_n,
      sclk_out                  => AD4020_spi_sclk_delayed
      );
      
  -- State machine that waits for a valid pulse from the FPGA, assumes the 
  -- system clock is faster or equal to the SPI clock
  process(sys_clk,sys_reset_n)
  begin
    if rising_edge(sys_clk) then 
      if ((AD4020_valid_in = '1' and AD4020_spi_busy = '0' and data_ready = '0') 
        or (data_ready = '1' and AD4020_spi_busy = '0')) then 
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
          if AD4020_spi_busy = '1' then 
            state <= spi_write_init_busy;
          else
            state <= spi_write_init_start;
          end if;
          
        when spi_write_init_busy =>
          if AD4020_spi_busy = '1' then 
            state <= spi_write_init_busy;
          else
            state <= spi_data_load;
          end if; 
          
        when spi_data_load => 
          state <= spi_read_data_start;

        when spi_read_data_start =>
          if AD4020_spi_busy = '1' then 
            state <= spi_read_data_busy;
          else
            state <= spi_read_data_start;
          end if;
          
        when spi_read_data_busy =>
        if AD4020_spi_busy = '1' then 
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
          AD4020_spi_command              <= WREN & write_data & control_reg;
          AD4020_spi_register_address     <= reg_config_reserved(15 downto 8);
          AD4020_spi_write_data 			    <= reg_config_reserved(7 downto 0);
          
        when spi_write_init_start =>
          AD4020_spi_write_data <= '1';
          
        when spi_write_init_busy =>
          AD4020_spi_write_data <= '0';
      
        when spi_data_load =>
          AD4020_conv <= '1';
          
          -- Workaround for transferring 24 data bits 
          AD4020_spi_command              <= write_data & control_reg;
          AD4020_spi_register_address     <= reg_config_reserved(15 downto 8);
          AD4020_spi_write_data 			    <= reg_config_reserved(7 downto 0);
          
        when spi_read_data_start =>
          AD4020_conv <= '0';
          AD4020_valid <= '0';
          
        when spi_read_data_busy =>
          AD4020_spi_write_data <= '0';
          
        when spi_read_data_finish =>
          AD4020_valid <= '1';
          AD4020_data <= AD4020_spi_read_data;
        
        when others => 
        -- do nothing
      end case;
    end if;
  end process;
  

  -- Map the output signals
  AD4020_SCLK_out <= AD4020_spi_sclk_delayed;
  AD4020_CONV_out <= AD4020_conv;
  AD4020_data_out <= "0000" & AD4020_data "00000000";
  
  

end architecture rtl; -- of FE_AD4020_v1






































