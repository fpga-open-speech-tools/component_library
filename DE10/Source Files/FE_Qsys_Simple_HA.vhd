----------------------------------------------------------------------------------
-- Company:          Flat Earth Inc
-- Author/Engineer:	 Tyler Davis
-- 
-- Create Date:    11/07/2018 
-- Design Name: 
-- Module Name:    FE_Qsys_Simple_HA
-- Project Name: 
-- Target Devices: DE10
-- Tool versions: 
-- Description: Simple hearing aid block that reads gain values for left and right 
--              channels.  The hearing aid used for this code was designed using an  
--              audio clock of 1.152 MHz and a system clock (sys_clk). To achieve
--              the audio clock rate, a clock of 73.728 MHz (6*mclk; audio_pll_clk)
--              is passed in, then divided (12.288*6/64). 
--
-- Dependencies: 
--
-- Revision: 
-- Revision 0.01 - File Created
-- Additional Comments: 
--
----------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

entity FE_Qsys_Simple_HA is
	port (
    
    ------------------------------------------------------------
    -- Avalon Memory Mapped Slave Signals
    ------------------------------------------------------------
		avalon_slave_address   : in  std_logic_vector(3 downto 0)  := (others => '0');
		avalon_slave_read      : in  std_logic                     := '0';
		avalon_slave_write     : in  std_logic                     := '0';
		avalon_slave_writedata : in  std_logic_vector(31 downto 0) := (others => '0');
		avalon_slave_readdata  : out std_logic_vector(31 downto 0);
		
    ------------------------------------------------------------
    -- Left Data Channel Input
    ------------------------------------------------------------
    data_in_left_data      : in  std_logic_vector(31 downto 0) := (others => '0');
		data_in_left_error     : in  std_logic_vector(1 downto 0)  := (others => '0');
		data_in_left_valid     : in  std_logic                     := '0';
    
    ------------------------------------------------------------
    -- Right Data Channel Input
    ------------------------------------------------------------
		data_in_right_data     : in  std_logic_vector(31 downto 0) := (others => '0');
		data_in_right_error    : in  std_logic_vector(1 downto 0)  := (others => '0');
		data_in_right_valid    : in  std_logic                     := '0';
    
    ------------------------------------------------------------
    -- Left Data Channel Outuput
    ------------------------------------------------------------
		data_out_left_data     : out std_logic_vector(31 downto 0);
		data_out_left_error    : out std_logic_vector(1 downto 0);
		data_out_left_valid    : out std_logic;
    
    ------------------------------------------------------------
    -- Right Data Channel Output
    ------------------------------------------------------------
		data_out_right_data    : out std_logic_vector(31 downto 0);
		data_out_right_error   : out std_logic_vector(1 downto 0);
		data_out_right_valid   : out std_logic;
    
    ------------------------------------------------------------
    -- Clocks and Resets
    ------------------------------------------------------------    
		sys_reset_n            : in  std_logic                     := '0';
		sys_clk                : in  std_logic                     := '0';
    audio_pll_clk          : in  std_logic                     := '0'  -- Must be 73.728 MHz
	);
end entity FE_Qsys_Simple_HA;

architecture rtl of FE_Qsys_Simple_HA is

  component HA_LR is
    port( 
      clk                               :   in    std_logic;
      reset                             :   in    std_logic;
      clk_enable                        :   in    std_logic;
      HA_left_data_in                   :   in    std_logic_vector(31 downto 0);  
      Gain_B1_left                      :   in    std_logic_vector(31 downto 0);  
      Gain_B2_left                      :   in    std_logic_vector(31 downto 0);  
      Gain_B3_left                      :   in    std_logic_vector(31 downto 0);  
      Gain_B4_left                      :   in    std_logic_vector(31 downto 0);  
      Gain_B5_left                      :   in    std_logic_vector(31 downto 0);  
      HA_right_data_in                  :   in    std_logic_vector(31 downto 0);  
      Gain_B1_right                     :   in    std_logic_vector(31 downto 0);  
      Gain_B2_right                     :   in    std_logic_vector(31 downto 0);  
      Gain_B3_right                     :   in    std_logic_vector(31 downto 0);  
      Gain_B4_right                     :   in    std_logic_vector(31 downto 0);  
      Gain_B5_right                     :   in    std_logic_vector(31 downto 0);  
      ce_out                            :   out   std_logic;
      HA_left_data_out                  :   out   std_logic_vector(31 downto 0);  
      HA_right_data_out                 :   out   std_logic_vector(31 downto 0)  
      );
  end component;
  
  component FE_Qsys_HA_DRC is
	port (
    clk 			    	        : in std_logic;   
    reset_n 		    	      : in std_logic;
    ------------------------------------------------------------
    -- Avalon Memory Mapped Slave Signals
    ------------------------------------------------------------
    avs_s1_address 	        : in  std_logic_vector( 3 downto 0);
    avs_s1_write 		        : in  std_logic;
    avs_s1_writedata 	      : in  std_logic_vector(31 downto 0);
    avs_s1_read 		        : in  std_logic;
    avs_s1_readdata 	      : out std_logic_vector(31 downto 0);
      
    ------------------------------------------------------------
    -- Left And Right Gain Signals
    ------------------------------------------------------------
    band1_gain_left             : out std_logic_vector(31 downto 0);
    band2_gain_left             : out std_logic_vector(31 downto 0);
    band3_gain_left             : out std_logic_vector(31 downto 0);
    band4_gain_left             : out std_logic_vector(31 downto 0);
    band5_gain_left             : out std_logic_vector(31 downto 0);
    band1_gain_right            : out std_logic_vector(31 downto 0);
    band2_gain_right            : out std_logic_vector(31 downto 0);
    band3_gain_right            : out std_logic_vector(31 downto 0);
    band4_gain_right            : out std_logic_vector(31 downto 0);
    band5_gain_right            : out std_logic_vector(31 downto 0)
	);
  end component;
  
  ------------------------------------------------------------------
  -- Input signal mappings
  ------------------------------------------------------------------
  signal data_in_left_data_r      :  std_logic_vector(31 downto 0);
  signal data_in_right_data_r     :  std_logic_vector(31 downto 0);
  signal data_out_left_data_r     :  std_logic_vector(31 downto 0);
  signal data_out_right_data_r    :  std_logic_vector(31 downto 0);
  
  ------------------------------------------------------------------
  -- HA_DRC signals
  ------------------------------------------------------------------
  signal band1_gain_left_r              : std_logic_vector(31 downto 0);
  signal band2_gain_left_r              : std_logic_vector(31 downto 0);
  signal band3_gain_left_r              : std_logic_vector(31 downto 0);
  signal band4_gain_left_r              : std_logic_vector(31 downto 0);
  signal band5_gain_left_r              : std_logic_vector(31 downto 0);
  signal band1_gain_right_r             : std_logic_vector(31 downto 0);
  signal band2_gain_right_r             : std_logic_vector(31 downto 0);
  signal band3_gain_right_r             : std_logic_vector(31 downto 0);
  signal band4_gain_right_r             : std_logic_vector(31 downto 0);
  signal band5_gain_right_r             : std_logic_vector(31 downto 0); 
  
  ------------------------------------------------------------------
  -- Clock Divider Signals
  ------------------------------------------------------------------
  signal audio_clk_counter              : unsigned(6 downto 0);
  signal HA_clk                         : std_logic;
      
begin

  u_HA: HA_LR port map ( 
      clk                               => HA_clk,
      reset                             => not sys_reset_n,
      clk_enable                        => '1', 
      HA_left_data_in                   => data_in_left_data_r, 
      Gain_B1_left                      => band1_gain_left_r,   
      Gain_B2_left                      => band2_gain_left_r,   
      Gain_B3_left                      => band3_gain_left_r,   
      Gain_B4_left                      => band4_gain_left_r,   
      Gain_B5_left                      => band5_gain_left_r,   
      HA_right_data_in                  => data_in_right_data_r,  
      Gain_B1_right                     => band1_gain_right_r, 
      Gain_B2_right                     => band2_gain_right_r, 
      Gain_B3_right                     => band3_gain_right_r, 
      Gain_B4_right                     => band4_gain_right_r, 
      Gain_B5_right                     => band5_gain_right_r, 
      ce_out                            => open,
      HA_left_data_out                  => data_out_left_data_r,
      HA_right_data_out                 => data_out_right_data_r
  );
  
  u_DRC: FE_Qsys_HA_DRC port map (
    clk 			    	        => sys_clk,   
    reset_n 		    	      => sys_reset_n,
    avs_s1_address 	        => avalon_slave_address,
    avs_s1_write 		        => avalon_slave_write,
    avs_s1_writedata 	      => avalon_slave_writedata,
    avs_s1_read 		        => avalon_slave_read,
    avs_s1_readdata 	      => avalon_slave_readdata,
      
    band1_gain_left         => band1_gain_left_r,
    band2_gain_left         => band2_gain_left_r,
    band3_gain_left         => band3_gain_left_r,
    band4_gain_left         => band4_gain_left_r,
    band5_gain_left         => band5_gain_left_r,
    band1_gain_right        => band1_gain_right_r,
    band2_gain_right        => band2_gain_right_r,
    band3_gain_right        => band3_gain_right_r,
    band4_gain_right        => band4_gain_right_r,
    band5_gain_right        => band5_gain_right_r
	);
  
  ---------------------------------------------------------------------
  -- Data in process
  ---------------------------------------------------------------------
  process(sys_clk)
  begin
    if (rising_edge(sys_clk)) then 
      if (data_in_left_valid = '1') then 
        data_in_left_data_r <= data_in_left_data;
      elsif (data_in_right_valid = '1') then 
        data_in_right_data_r <= data_in_right_data;
      end if; -- valid signals
    end if; -- rising clock
  end process;
  
  -------------------------------------------------------------
	-- Clock Divider Process
	-------------------------------------------------------------
	process(sys_reset_n, audio_pll_clk)
	begin
        if sys_reset_n = '0' then
            audio_clk_counter <= (others => '0');
        elsif rising_edge(audio_pll_clk) then
            audio_clk_counter <= audio_clk_counter + 1;
 		end if;
	end process;
  
    
  ---------------------------------------------------------------------
  -- Audio Clock Mapping
  ---------------------------------------------------------------------
  HA_clk <= not audio_clk_counter(5);
  
  ---------------------------------------------------------------------
  -- Data out mapping
  -- Note: This follows the previous project mapping conventions
  ---------------------------------------------------------------------
  data_out_left_data    <= data_out_left_data_r;
  data_out_left_valid   <= data_in_left_valid;
  data_out_left_error   <= data_in_left_error;
  data_out_right_data   <= data_out_right_data_r;
  data_out_right_valid  <= data_in_right_valid;
  data_out_right_error  <= data_in_right_error;
  
end architecture rtl;































