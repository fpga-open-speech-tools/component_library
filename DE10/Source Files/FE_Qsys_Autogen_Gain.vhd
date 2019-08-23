library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

entity FE_Qsys_Autogen_Gain is
	port (
    
    ------------------------------------------------------------
    -- Avalon Memory Mapped Slave Signals
    ------------------------------------------------------------
		avalon_slave_address   : in  std_logic_vector(0 downto 0)  := (others => '0');
		avalon_slave_read      : in  std_logic                     := '0';
		avalon_slave_write     : in  std_logic                     := '0';
		avalon_slave_writedata : in  std_logic_vector(31 downto 0) := (others => '0');
		avalon_slave_readdata  : out std_logic_vector(31 downto 0);
		
    ------------------------------------------------------------
    -- Data Channel input
    ------------------------------------------------------------
    data_in_data      : in  std_logic_vector(31 downto 0) := (others => '0');
		data_in_error     : in  std_logic_vector(1 downto 0)  := (others => '0');
		data_in_valid     : in  std_logic                     := '0';
    data_in_channel   : in  std_logic_vector(1 downto 0)  := "11";

    ------------------------------------------------------------
    -- Data Channel Output
    ------------------------------------------------------------
    data_out_data      : out  std_logic_vector(31 downto 0);
		data_out_error     : out  std_logic_vector(1 downto 0);
		data_out_valid     : out  std_logic;
    data_out_channel   : out  std_logic_vector(1 downto 0);
    
    ------------------------------------------------------------
    -- Clocks and Resets
    ------------------------------------------------------------    
		sys_reset_n            : in  std_logic                     := '0';
		sys_clk                : in  std_logic                     := '0'
	);
end entity FE_Qsys_Autogen_Gain;

architecture rtl of FE_Qsys_Autogen_Gain is

component DataPlane_dut IS
  port( clk                               :   in    std_logic;
        reset                             :   in    std_logic;
        dut_enable                        :   in    std_logic;  -- ufix1
        Avalon_Sink_Valid                 :   in    std_logic;  -- ufix1
        Avalon_Sink_Data                  :   in    std_logic_vector(31 downto 0);  -- sfix32_En28
        Avalon_Sink_Channel               :   in    std_logic_vector(1 downto 0);  -- ufix2
        Avalon_Sink_Error                 :   in    std_logic_vector(1 downto 0);  -- ufix2
        Register_Control_Left_Gain        :   in    std_logic_vector(31 downto 0);  -- sfix32_En28
        Register_Control_Right_Gain       :   in    std_logic_vector(31 downto 0);  -- sfix32_En28
        ce_out                            :   out   std_logic;  -- ufix1
        Avalon_Source_Valid               :   out   std_logic;  -- ufix1
        Avalon_Source_Data                :   out   std_logic_vector(31 downto 0);  -- sfix32_En28
        Avalon_Source_Channel             :   out   std_logic_vector(1 downto 0);  -- ufix2
        Avalon_Source_Error               :   out   std_logic_vector(1 downto 0)  -- ufix2
        );
end component;
  
component FE_Qsys_Autogen_Gainv1 is
	port (
    clk 			    	        : in std_logic;   
    reset_n 		    	      : in std_logic;
    ------------------------------------------------------------
    -- Avalon Memory Mapped Slave Signals
    ------------------------------------------------------------
    avs_s1_address 	        : in  std_logic_vector( 0 downto 0);   --! Avalon MM Slave address
    avs_s1_write 		        : in  std_logic;                       --! Avalon MM Slave write
    avs_s1_writedata 	      : in  std_logic_vector(31 downto 0);   --! Avalon MM Slave write data
    avs_s1_read 		        : in  std_logic;                       --! Avalon MM Slave read
    avs_s1_readdata 	      : out std_logic_vector(31 downto 0);   --! Avalon MM Slave read data
    ------------------------------------------------------------
    -- Exported control words
    ------------------------------------------------------------       
    gain_left               : out std_logic_vector(31 downto 0);
    gain_right              : out std_logic_vector(31 downto 0)
	);
end component;
  
  ------------------------------------------------------------------
  -- input signal mappings
  ------------------------------------------------------------------
  signal data_in_data_r       :  std_logic_vector(31 downto 0);
  signal data_out_data_r      :  std_logic_vector(31 downto 0);
  signal data_in_channel_r    :  std_logic_vector(1 downto 0);
  signal data_out_channel_r   :  std_logic_vector(1 downto 0);
  signal data_in_valid_r      : std_logic;
  signal data_out_valid_r     : std_logic;
  signal data_in_error_r      : std_logic_vector(1 downto 0);
  signal data_out_error_r     : std_logic_vector(1 downto 0);
  
  ------------------------------------------------------------------
  -- HA_DRC signals
  ------------------------------------------------------------------
  signal gain_left_r              : std_logic_vector(31 downto 0);
  signal gain_right_r              : std_logic_vector(31 downto 0);
  
      
begin

  u_DP: DataPlane_dut port map ( 
    clk                               => sys_clk,
    reset                             => not sys_reset_n,
    dut_enable                        => '1',
    Avalon_Sink_Valid                 => data_in_valid,
    Avalon_Sink_Data                  => data_in_data_r,
    Avalon_Sink_Channel               => data_in_channel,
    Avalon_Sink_Error                 => data_in_error,
    Register_Control_Left_Gain        => gain_left_r,
    Register_Control_Right_Gain       => gain_right_r,
    ce_out                            => open,
    Avalon_Source_Valid               => data_out_valid_r,
    Avalon_Source_Data                => data_out_data_r,
    Avalon_Source_Channel             => data_out_channel_r,
    Avalon_Source_Error               => data_out_error_r
  );
  
  u_AG: FE_Qsys_Autogen_Gainv1 port map (
    clk 			    	        => sys_clk,   
    reset_n 		    	      => sys_reset_n,
    avs_s1_address 	        => avalon_slave_address,
    avs_s1_write 		        => avalon_slave_write,
    avs_s1_writedata 	      => avalon_slave_writedata,
    avs_s1_read 		        => avalon_slave_read,
    avs_s1_readdata 	      => avalon_slave_readdata,
      
    gain_left         => gain_left_r,
    gain_right         => gain_right_r
	);
  
  ---------------------------------------------------------------------
  -- Data in process
  ---------------------------------------------------------------------
  process(sys_clk)
  begin
    if (rising_edge(sys_clk)) then 
      if (data_in_valid = '1') then 
        data_in_data_r <= data_in_data;
      else
        data_in_data_r <= data_in_data_r;
      end if; -- valid signals
    end if; -- rising clock
  end process;
  
  
  ---------------------------------------------------------------------
  -- Data out mapping
  -- Note: This follows the previous project mapping conventions
  ---------------------------------------------------------------------
  data_out_data    <= data_out_data_r;
  data_out_valid   <= data_in_valid;
  data_out_error   <= data_in_error_r;
  data_out_channel   <= data_out_channel_r;
  
end architecture rtl;































