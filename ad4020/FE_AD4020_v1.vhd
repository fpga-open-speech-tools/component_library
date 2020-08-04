----------------------------------------------------------------------------
--! @file FE_AD4020_v1.vhd
--! @brief Implements serial/streaming data transfer for the AD4020 ADC.
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

entity FE_AD4020_v1 is
  generic(
    input_clk_freq        : natural range 1 to 1_000_000_000 := 72_000_000 --input clock speed from user logic in Hz
  );
	port (
    sys_clk               : in  std_logic;                        --      clk_in.clk_in
	  sys_reset_n           : in  std_logic;                        --      reset.reset_n

    spi_clk               : in  std_logic;  -- REMOVE_COMMENT: currently set to 71.4285 Mhz

    -----------------------------------------------------------------------------------------------------------
    -- Abstracted data channels, i.e. interface to the data plane as 20-bit data words.
    -----------------------------------------------------------------------------------------------------------
    AD4020_data_out        : out  std_logic_vector(31 downto 0);     --      data in
    AD4020_valid_out       : out  std_logic;
    AD4020_error_out       : out  std_logic_vector(1 downto 0);
    AD4020_channel_out     : out  std_logic_vector(1 downto 0);
    
    -------------------------------------------------------------------------------------------------------------------------------------
    -- AD4020 Physical Layer : Signals to/from AD4020 Serial Data Port (from ADCs/to DACs), i.e. connection to physical pins on AD4020
    -------------------------------------------------------------------------------------------------------------------------------------
    AD4020_MISO_in        : in  std_logic;                        --      MISO for the SPI line
    AD4020_MOSI_out       : out std_logic;                        --      MISO for the SPI line
    AD4020_SCLK_out       : out std_logic;
    AD4020_CONV_out       : out std_logic
	);
end entity FE_AD4020_v1;

architecture rtl of FE_AD4020_v1 is

  -- Declare the SPI component
  component spi_commands is
	  generic(
	
	  command_used_g          : std_logic 	:= '1';
	  address_used_g          : std_logic 	:= '0';
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
  signal   AD4020_spi_command       	  : std_logic_vector(1 downto 0);  
  signal   AD4020_spi_register_address  : std_logic_vector(5 downto 0);  
  signal   AD4020_spi_write_data        : std_logic_vector(7 downto 0);   -- data to be written to AD4020 register
  signal   AD4020_spi_write_data_rdy    : std_logic;                      -- assert (clock pulse) to write data
  signal   AD4020_spi_busy          	  : std_logic;                      -- If 1, the spi is busy servicing a command. Wait until 0 to send another command. 
  signal   AD4020_spi_sclk              : std_logic;
  signal   AD4020_spi_sclk_delayed      : std_logic;
  signal   AD4020_conv                  : std_logic;
  signal   AD4020_valid                 : std_logic := '0';
  signal   AD4020_data                  : std_logic_vector(19 downto 0) := "00000000000000000000";
  
  -- Data "addresses"
  constant control_reg                    : std_logic_vector(5 downto 0) := "010100";
  
  -- Data read and write commands
  constant write_data                     : std_logic := '0';
  constant read_data                      : std_logic := '1';
  signal data_ready                     : std_logic := '0';
    
  -- Command register bits defined on pg. 25 of the manual
  constant WEN                            : std_logic := '0';
  constant RESERVED                       : std_logic := '0'; -- Reserved bits should be programmed as zeros
  constant TURBO                          : std_logic := '1'; -- Default to disabled
  constant OV                             : std_logic := '0'; -- Default to disabled
  constant HIGHZ                          : std_logic := '0'; -- Default to disabled
  constant SPAN_COMPRESSION               : std_logic := '0'; -- Default to disabled
  constant ENABLE_STATUS_BITS             : std_logic := '0'; -- Default to disabled
  
  --signal RDOP                           : std_logic_vector(7 downto 0) := "00000000";
  
  constant reg_config                     : std_logic_vector(7 downto 0)  := RESERVED & RESERVED & RESERVED & ENABLE_STATUS_BITS &
                                                                           SPAN_COMPRESSION & HIGHZ & TURBO & OV;
                                                                           
  -- ADC init flag
  signal init_done                      : std_logic := '0'; 
  
  type state_type is (
    init_adc,
    spi_init_load,
    spi_data_load,
    spi_read_data_busy,
    spi_read_data_start,
    spi_write_init_busy,
    spi_write_init_start,
    spi_read_data_finish
  );
  
  signal state : state_type;
  
  signal sclk_en : boolean := true;
  signal spi_clk_en : boolean := false;
  signal read_complete : boolean := false;
  
  signal one_cycle_delay : boolean := false;
  signal SDI_is_low : boolean := true;
  signal tquiet1_delay_complete : boolean := false;
  signal tquiet2_delay_complete : boolean := false;
  signal gpio_init_complete : boolean := false;
  
  signal mosi_ctrl : boolean := true;
  signal spi_mosi  : std_logic;
  signal read_mosi : std_logic;
  
begin

  -----------------------------------------------------------------------
  -- SPI interface to the AD5791 SPI Control Port
  -----------------------------------------------------------------------
  spi_AD4020: spi_commands
  generic map (
    command_used_g            => '1',  -- command field is used
    address_used_g            => '1',  -- address field is used
    command_width_bits_g      =>  2,   -- command is 2 bits: WREN & R/W
    address_width_bits_g      =>  6,   -- address is 6 bits: concatenated with command
    data_width_bits_g 	      =>  8,
    output_bits_g             =>  16,  -- not actually reading from spi component
    cpol_cpha                 => "00"  -- AD4020:  CPOL=0, CPHA=0  This is actually CPOL=0, CPHA=1 as implemented
  )
  port map (
    clk	                      => spi_clk    ,  					-- spi clock (50 MHz max)
    rst_n 	                  => sys_reset_n,		   				    -- component reset
    
    command_in                => AD4020_spi_command,  				-- Command includes Global Address (0000100) and is either Read ("00001001") or Write ("00001000").
    address_in                => AD4020_spi_register_address,  	-- Register Address.  
    
    master_slave_data_in      => AD4020_spi_write_data,			-- data to be written to an AD5791 register
    master_slave_data_rdy_in  => AD4020_spi_write_data_rdy,    	-- assert (clock pulse) to write the data
    master_slave_data_ack_out => open,                         	-- ignore acknowledgement 
    command_busy_out          => AD4020_spi_busy,					-- If 1, the spi is busy servicing a command. 
    command_done              => open,					-- pulse signals end of command
    
    slave_master_data_out     => open,				-- data read from AD5791 register
    slave_master_data_ack_out => open,		-- data ready to be read
    
    miso 				              => AD4020_MISO_in,					-- AD5791 SPI signal = data from AD5791 SPI registers
    mosi 					            => spi_mosi,						-- AD5791 SPI signal = data to AD5791 SPI registers
    sclk 					            => AD4020_spi_sclk,					-- AD5791 SPI signal = sclk: serial clock
    cs_n 					            => open				-- AD5791 SPI signal = ss_n: slave select (active low)
  );
  
  -- State machine that handles the state transitions 
  process (spi_clk, sys_reset_n)
  begin
  
    -- Reset the signals when the system reset is deasserted
    if sys_reset_n = '0' then
      state     <= init_ADC;  
      init_done <= '0';
      
    -- If the reset is not asserted, 
    elsif (rising_edge(spi_clk)) then
      --state_follower <= state;
      case state is  
        when init_ADC =>
          if init_done = '0' then 
            if gpio_init_complete then 
            state <= spi_init_load;
            init_done <= '1';
            end if;
          elsif init_done = '1' then 
            state <= spi_read_data_start;
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
            state <= spi_read_data_start;
            mosi_ctrl <= false;
            sclk_en <= false;
          end if; 
        when spi_read_data_start =>
          if one_cycle_delay then 
            state <= spi_read_data_busy;
          else
            state <= spi_read_data_start;
          end if;
          
        when spi_read_data_busy =>
          if read_complete then 
            state <= spi_read_data_finish;
          else
            state <= spi_read_data_busy;
          end if;   

        when spi_read_data_finish =>
          if tquiet2_delay_complete then
            state <= spi_read_data_start;
          end if;
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
    variable bits_left : integer := 20;
    
  begin
    if (rising_edge(spi_clk)) then
      case state is     
      
        when init_ADC =>
      
        when spi_init_load =>
          AD4020_conv                     <= '1';
          AD4020_spi_command              <= WEN & write_data;
          AD4020_spi_register_address     <= control_reg;
          AD4020_spi_write_data 			    <= reg_config;
          
        when spi_write_init_start =>
          AD4020_spi_write_data_rdy <= '1';
          AD4020_conv               <= '0';
          
        when spi_write_init_busy =>
          AD4020_spi_write_data_rdy <= '0';
        when spi_read_data_start =>
          -- SDI must be high before the rising edge of CNV
          if SDI_is_low then
            read_mosi <= '1';
          else 
            AD4020_conv <= '1';
          end if;
          AD4020_valid <= '0';

          if tquiet1_delay_complete then
            read_mosi <= '0';
            one_cycle_delay <= true;
            if one_cycle_delay then
              spi_clk_en <= true;
            end if;
          end if;
        when spi_read_data_busy =>
          one_cycle_delay <= false;
          if not(bits_left > 0) then
            spi_clk_en <= false;
          end if;
        when spi_read_data_finish =>
          AD4020_valid <= '1';
          if tquiet2_delay_complete then
            AD4020_conv <= '0';
          end if;
        
        when others => 
        -- do nothing
      end case;
    end if;
    if (falling_edge(spi_clk)) then
      case state is     
        when spi_read_data_busy =>
          if bits_left > 0 then
            AD4020_data(bits_left - 1) <= AD4020_MISO_in;
            bits_left := bits_left - 1;
          else 
            read_complete <= true;
          end if;
        when spi_read_data_finish =>
          read_complete <= false;
          bits_left := 20;
        
        when others => 
        -- do nothing
      end case;
    end if;
  end process;
  

  delays : process(spi_clk, state)
  constant tquiet1_ns : natural := 200;
  constant tquiet2_ns : natural := 60;
  constant gpio_init_ns : natural := 750;
  constant period_ns : natural range 1 to 1000 := 1_000_000_000 / input_clk_freq; 

  -- the number of cycles needed for the delays
  -- calculated by doing the ceiling of the delay / period
  constant tquiet1_cycles : natural := (tquiet1_ns + period_ns - 1) / period_ns; 
  constant tquiet2_cycles : natural := (tquiet2_ns + period_ns - 1) / period_ns;
  constant gpio_init_cycles : natural := (gpio_init_ns + period_ns - 1) / period_ns;

  variable counter : natural := 0;
  begin
    if rising_edge(spi_clk) then
      case state is
        when spi_read_data_start =>
          if SDI_is_low then
            SDI_is_low <= false;
          elsif counter = tquiet1_cycles or tquiet1_delay_complete then
            tquiet1_delay_complete <= true;
            counter := 0;
          else 
            counter := counter + 1;
          end if; 
        when init_ADC =>
          if counter = gpio_init_cycles or gpio_init_complete then
            gpio_init_complete <= true;
            counter := 0;
          else 
            counter := counter + 1;
          end if; 
        when spi_read_data_finish =>
          if counter = tquiet2_cycles or tquiet2_delay_complete then
            tquiet2_delay_complete <= true;
            counter := 0;
          else 
            counter := counter + 1;
          end if; 
        when others =>
          -- Reset attributes
          SDI_is_low <= true;
          counter := 0;
          tquiet1_delay_complete <= false;
          tquiet2_delay_complete <= false;
          gpio_init_complete <= false;
		end case;
    end if;
  end process;

  -- Map the output signals
  AD4020_SCLK_out <=AD4020_spi_sclk when sclk_en else spi_clk when spi_clk_en else '0';
  AD4020_CONV_out <= AD4020_conv;
  AD4020_data_out <= std_logic_vector(resize(signed(AD4020_data), AD4020_data_out'length));
  AD4020_MOSI_out <= spi_mosi when mosi_ctrl else read_mosi;
  AD4020_valid_out <= AD4020_valid;
  AD4020_channel_out <= "00";

end architecture rtl; -- of FE_AD4020_v1






































