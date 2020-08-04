----------------------------------------------------------------------------
--! @file FE_AD5791_v1.vhd
--! @brief Implements serial/streaming data transfer for the AD5791 DAC.
--! @details This 
--! @author Tyler Davis
--! @date 2019
--! @copyright Copyright 2019 Audio Logic
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

--! @brief Entity declaration for the AD5791 module
--! @param sys_clk System clock.  
--! @param sys_reset_in System reset.
--! @param spi_clk SPI clock
--! @param A clock double the SPI clock rate.  This is for 00 and 01 modes
--! @param AD5791_data_in 32 bit input data
--! @param AD5791_valid_in Avalon valid signal
--! @param AD5791_error_in Avalon error signal
--! @param AD5791_MISO_out Master in, slave out (SDO on DAC)
--! @param AD5791_MOSI_in Master out, slave in (SDI on DAC)
--! @param AD5791_LDAC_n_out DAC load DAC signal
--! @param AD5791_SYNC_n_out DAC data sync signal
--! @param AD5791_CLR_n_out DAC clear signal
--! @param AD5791_SCLK_out SPI sclk
entity FE_AD5791_v1 is
	port (
		sys_clk               : in  std_logic;                        --      clk_in.clk_in
		sys_reset_n           : in  std_logic;                        --      reset.reset_n

    spi_clk               : in  std_logic;
    double_spi_clk_in     : in  std_logic;

    -----------------------------------------------------------------------------------------------------------
    -- Abstracted data channels, i.e. interface to the data plane as 20-bit data words.
    -----------------------------------------------------------------------------------------------------------
    AD5791_data_in        : in  std_logic_vector(31 downto 0);     --      data in
    AD5791_valid_in       : in  std_logic;
    AD5791_error_in       : in  std_logic_vector(1 downto 0);
    
    -------------------------------------------------------------------------------------------------------------------------------------
    -- AD5791 Physical Layer : Signals to/from AD5791 Serial Data Port (from ADCs/to DACs), i.e. connection to physical pins on AD5791
    -------------------------------------------------------------------------------------------------------------------------------------
    AD5791_MISO_out       : in std_logic;                           --      MISO for the SPI line
    AD5791_MOSI_in        : out std_logic;                         --      MISO for the SPI line
    AD5791_LDAC_n_out     : out std_logic;
    AD5791_SYNC_n_out     : out std_logic;
    AD5791_CLR_n_out      : out std_logic;
    AD5791_SCLK_out       : out std_logic
	);
end entity FE_AD5791_v1;

architecture rtl of FE_AD5791_v1 is

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
  
  -- Add an additional block to make the SPI communication true "00" mode
  component spi_clk_delay is
    port (
      spi_clk                   : in  std_logic;
      double_spi_clk            : in  std_logic;
      sys_reset                 : in  std_logic;
      sclk_out                  : out  std_logic
      );
  end component;
  
   
  -----------------------------------------------------------------------------------------------------
	-- SPI related signals
  -----------------------------------------------------------------------------------------------------
  signal   AD5791_spi_command       	  : std_logic_vector(7 downto 0);  
  signal   AD5791_spi_register_address  : std_logic_vector(7 downto 0);  
  signal   AD5791_spi_write_data        : std_logic_vector(7 downto 0);   -- data to be written to AD5791 register
  signal   AD5791_spi_write_data_rdy    : std_logic;                      -- assert (clock pulse) to write data
  signal   AD5791_spi_busy          	  : std_logic;                      -- If 1, the spi is busy servicing a command. Wait until 0 to send another command. 
  signal   AD5791_spi_done          	  : std_logic;                      
  signal   AD5791_spi_read_data         : std_logic_vector(23 downto 0);   -- data read from AD5791 register
  signal   AD5791_spi_read_data_ack  	  : std_logic;                      -- data ready to be read
  signal   AD5791_spi_sclk              : std_logic;
  signal   AD5791_spi_sclk_delayed      : std_logic;
  
  -- Data "address"
  signal data_reg                       : std_logic_vector(2 downto 0) := "001";
  signal control_reg                    : std_logic_vector(2 downto 0) := "010";
  
  -- Data read and write commands
  signal write_data                     : std_logic := '0';
  signal read_data                      : std_logic := '1';
  signal ldac_hold                      : std_logic := '0';
  signal data_ready                     : std_logic := '0';
  
  -- Output signals
  signal ldac_out                       : std_logic := '1';
  signal sync_out                       : std_logic := '1';
  signal clr_out                        : std_logic := '1';
  
  -- Command register bits defined on pg. 22 of the manual
  signal RESERVED                       : std_logic                     := '0'; -- Reserved bits should be programmed as zeros
  signal RBUF                           : std_logic                     := '0'; -- Output amplifier config; default
  signal OPGND                          : std_logic                     := '0'; -- Output ground clamp; zero removes ground clamp
  signal DACTRI                         : std_logic                     := '0'; -- Tristate control; zero sets normal mode
  signal BIN2sC                         : std_logic                     := '0'; -- DAC regisger coding select; default
  signal SDODIS                         : std_logic                     := '0'; -- SDO pin enable/disable; default
  signal LINCOMP                        : std_logic_vector(3 downto 0)  := "0000"; -- Linearity error compensation; default
  
  signal reg_config                     : std_logic_vector(8 downto 0)  := LINCOMP & SDODIS & BIN2sC & DACTRI & OPGND & RBUF;
  signal reg_config_reserved            : std_logic_vector(19 downto 0) := "0000000000" & reg_config & RESERVED;
  
  -- DAC init flag
  signal init_done                      : std_logic := '0'; 
  
  type state_type is ( 
    init_DAC,
    spi_data_wait,
    spi_write_data_busy,
    spi_data_load,
    spi_write_data_start,
    spi_init_load,
    spi_cmd_read_load
  );
  
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
      spi_clk                   => AD5791_spi_sclk,
      double_spi_clk            => double_spi_clk_in,
      sys_reset                 => sys_reset_n,
      sclk_out                  => AD5791_spi_sclk_delayed
      );
  
  -- State machine that waits for a valid pulse from the FPGA, assumes the 
  -- system clock is faster or equal to the SPI clock
  process(sys_clk,sys_reset_n)
  begin
    if rising_edge(sys_clk) then 
      if ((AD5791_valid_in = '1' and AD5791_spi_busy = '0' and data_ready = '0') 
        or (data_ready = '1' and AD5791_spi_busy = '0')) then 
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
      state     <= init_DAC;  
      init_done <= '0';
      
    -- If the reset is not asserted, 
    elsif (rising_edge(spi_clk)) then
      case state is  
        when init_DAC =>
          -- If the DAC hasn't been initialized, move to the init state
          if init_done = '0' and AD5791_valid_in = '1' then
            state <= spi_init_load;
            init_done <= '1';
          -- Otherswise wait for data to arrived
          elsif init_done = '1' then
            state <= spi_data_wait;
          -- Debug catch all to avoid a latch
          else
            state <= init_DAC;
          end if;
          
      when spi_init_load => 
      -- Transition to the start of the data write immediately
        state <= spi_write_data_start;
      
			when spi_data_wait =>
        -- If data has arrived, transition to the load data state
				if ( data_ready = '1' ) then
					state <= spi_data_load;
        -- Otherwise, remain in the data wait state
        else
          state <= spi_data_wait;
				end if;
      
			when spi_data_load =>
        -- When valid data has arrived, transition to the start state
				state <= spi_write_data_start;
        
			when spi_cmd_read_load =>
      -- When a command has been loaded, transiition to the data start phase
				state <= spi_write_data_start;
        
			when spi_write_data_start =>
        -- Make sure the spi module is not busy before sending data
        if AD5791_spi_busy = '1' then 
          state <= spi_write_data_busy;
        -- If it is busy, stay in the data start state
        else
          state <= spi_write_data_start;
        end if;
        
			when spi_write_data_busy =>
        -- If the spi module is busy, remain in this state
				if AD5791_spi_busy = '1' then
					state <= spi_write_data_busy;
        -- Otherswise, the SPI module finished and is ready for more data
				else
				   state <= spi_data_wait;
				end if;	
        
        when others =>
          -- If an unexpected state occurred, wait for more data
          state <= spi_data_wait;
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
        when spi_init_load =>
          ldac_hold <= '1';
          
          -- Workaround for transferring 24 data bits 
          AD5791_spi_command              <= write_data & control_reg & reg_config_reserved(19 downto 16);
          AD5791_spi_register_address     <= reg_config_reserved(15 downto 8);
          AD5791_spi_write_data 			    <= reg_config_reserved(7 downto 0);
          
        when spi_cmd_read_load =>
          ldac_hold <= '1';
          
          -- Workaround for transferring 24 data bits 
          AD5791_spi_command              <= read_data & control_reg & "0000";
          AD5791_spi_register_address     <= "00000000";
          AD5791_spi_write_data 			    <= "00000000";
                    
        when spi_data_wait =>
        
          -- Wait for one clock cycle after the sync goes high before 
          -- pulsing the LDAC line
          if ldac_hold = '1' then 
            ldac_out <= '0';
            ldac_hold <= '0';
          else 
            ldac_out <= '1';
          end if;
          
          -- Set the sync to "1" before pulsing the LDAC
          sync_out <= '1';
        
        when spi_data_load =>
        
          -- Set the flag to pulse the LDAC one clock cycle after SYNC transition
          ldac_hold <= '1';
          -- Workaround for transferring 24 data bits           
          AD5791_spi_command              <= "10101010";--write_data & data_reg & AD5791_data_in(30 downto 27);
          AD5791_spi_register_address     <= "01011100";--AD5791_data_in(26 downto 19);
          AD5791_spi_write_data 			    <= "01000101";--AD5791_data_in(18 downto 11);
          
        when spi_write_data_start =>
          -- Signal the data for the SPI module is ready
          AD5791_spi_write_data_rdy   <= '1';
          -- Deassert sync to load data into the DAC shift register
          sync_out <= '0';
          
        when spi_write_data_busy =>
          -- Deassert the data ready signal after the data writing starts
          AD5791_spi_write_data_rdy <= '0';
          
          -- When the SPI module is no longer busy, assert the sync to "finish"
          -- loading the data, otherwise hold the sync low to continue loading
          -- data into the shift register.
          if AD5791_spi_busy = '0' then 
            sync_out <= '1';
          else 
            sync_out <= '0';
          end if;
        
        when others => 
        -- do nothing
      end case;
    end if;
  end process;

  -- Map the output signals
  AD5791_SCLK_out <= AD5791_spi_sclk_delayed;
  AD5791_CLR_n_out <= clr_out;
  AD5791_LDAC_n_out <= ldac_out;
  AD5791_SYNC_n_out <= sync_out;
  

end architecture rtl; -- of FE_AD5791_v1






































