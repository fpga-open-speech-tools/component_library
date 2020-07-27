----------------------------------------------------------------------------------
-- Company:          Audio Logic
-- Author/Engineer:	 Tyler Davis (based on a design by Ross Snider)
-- 
-- Create Date:    11/07/2018 
-- Design Name: 
-- Module Name: FE_Qsys_HA_Simple_Gainv8
-- Project Name: 
-- Target Devices: DE10
-- Tool versions: 
-- Description:     Hearing Aid Qsys Block with DRC that exports control to top level 
--
-- Dependencies: 
--
-- Revision: 
-- Revision 0.01 - File Created
-- Additional Comments: 
--
----------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity FE_Qsys_HA_Simple_Gainv8 is
	port (
    clk 			    	        : in std_logic;   
    reset_n 		    	      : in std_logic;
    ------------------------------------------------------------
    -- Avalon Memory Mapped Slave Signals
    ------------------------------------------------------------
    avs_s1_address 	        : in  std_logic_vector( 3 downto 0);   --! Avalon MM Slave address
    avs_s1_write 		        : in  std_logic;                       --! Avalon MM Slave write
    avs_s1_writedata 	      : in  std_logic_vector(31 downto 0);   --! Avalon MM Slave write data
    avs_s1_read 		        : in  std_logic;                       --! Avalon MM Slave read
    avs_s1_readdata 	      : out std_logic_vector(31 downto 0);   --! Avalon MM Slave read data
    ------------------------------------------------------------
    -- Exported control words
    ------------------------------------------------------------       
    band1_gain_left             : out std_logic_vector(31 downto 0);
    band2_gain_left             : out std_logic_vector(31 downto 0);
    band3_gain_left             : out std_logic_vector(31 downto 0);
    band4_gain_left             : out std_logic_vector(31 downto 0);
    gain_all_left               : out std_logic_vector(31 downto 0);
    band1_gain_right            : out std_logic_vector(31 downto 0);
    band2_gain_right            : out std_logic_vector(31 downto 0);
    band3_gain_right            : out std_logic_vector(31 downto 0);
    band4_gain_right            : out std_logic_vector(31 downto 0);
    gain_all_right              : out std_logic_vector(31 downto 0)
	);
end FE_Qsys_HA_Simple_Gainv8;

architecture behavior of FE_Qsys_HA_Simple_Gainv8 is

  signal band1_gain_left_r      : std_logic_vector(31 downto 0);
  signal band2_gain_left_r      : std_logic_vector(31 downto 0);
  signal band3_gain_left_r      : std_logic_vector(31 downto 0);
  signal band4_gain_left_r      : std_logic_vector(31 downto 0);
  signal gain_all_left_r      : std_logic_vector(31 downto 0);
  signal band1_gain_right_r     : std_logic_vector(31 downto 0);
  signal band2_gain_right_r     : std_logic_vector(31 downto 0);
  signal band3_gain_right_r     : std_logic_vector(31 downto 0);
  signal band4_gain_right_r     : std_logic_vector(31 downto 0);
  signal gain_all_right_r     : std_logic_vector(31 downto 0);


begin

    ------------------------------------------------------------------------
    -- Read from Registers
    ------------------------------------------------------------------------ 
    process(clk)
    begin
      if rising_edge(clk) and (avs_s1_read = '1') then  -- all registers can be read. 
        case avs_s1_address is
          when "0000"  => avs_s1_readdata <= gain_all_left_r;
          when "0001"  => avs_s1_readdata <= band1_gain_left_r; 
          when "0010"  => avs_s1_readdata <= band2_gain_left_r;         
          when "0011"  => avs_s1_readdata <= band3_gain_left_r;
          when "0100"  => avs_s1_readdata <= band4_gain_left_r; 
          when "1000"  => avs_s1_readdata <= gain_all_right_r;
          when "1001"  => avs_s1_readdata <= band1_gain_right_r; 
          when "1010"  => avs_s1_readdata <= band2_gain_right_r;         
          when "1011"  => avs_s1_readdata <= band3_gain_right_r;
          when "1100"  => avs_s1_readdata <= band4_gain_right_r; 
          when others  => avs_s1_readdata <= (others => '0');
         end case;
      end if;
    end process;

    
    
    ------------------------------------------------------------------------
    -- Write to Registers
    ------------------------------------------------------------------------ 
    process(clk)
    begin
          if (reset_n = '0') then
            band1_gain_left_r     <= x"00010000";  -- W32F16
            band2_gain_left_r     <= x"00010000";  -- W32F16
            band3_gain_left_r     <= x"00010000";  -- W32F16
            band4_gain_left_r     <= x"00010000";  -- W32F16
            gain_all_left_r       <= x"00010000";  -- W32F16
            band1_gain_right_r    <= x"00010000";  -- W32F16
            band2_gain_right_r    <= x"00010000";  -- W32F16
            band3_gain_right_r    <= x"00010000";  -- W32F16
            band4_gain_right_r    <= x"00010000";  -- W32F16
            gain_all_right_r      <= x"00010000";  -- W32F16
      elsif rising_edge(clk) and (avs_s1_write = '1') then  -- write the registers
        case avs_s1_address is
          when "0000"  => gain_all_left_r             <= avs_s1_writedata;
          when "0001"  => band1_gain_left_r           <= avs_s1_writedata;
          when "0010"  => band2_gain_left_r           <= avs_s1_writedata;            
          when "0011"  => band3_gain_left_r           <= avs_s1_writedata;
          when "0100"  => band4_gain_left_r           <= avs_s1_writedata;
          when "1000"  => gain_all_right_r            <= avs_s1_writedata;
          when "1001"  => band1_gain_right_r          <= avs_s1_writedata;
          when "1010"  => band2_gain_right_r          <= avs_s1_writedata;            
          when "1011"  => band3_gain_right_r          <= avs_s1_writedata;
          when "1100"  => band4_gain_right_r          <= avs_s1_writedata;
          when others  => null;                     
        end case;
      end if;    
    end process;
      
    band1_gain_left           <= band1_gain_left_r;
    band2_gain_left           <= band2_gain_left_r;  
    band3_gain_left           <= band3_gain_left_r;  
    band4_gain_left           <= band4_gain_left_r;  
    gain_all_left             <= gain_all_left_r;  

    band1_gain_right          <= band1_gain_right_r;
    band2_gain_right          <= band2_gain_right_r;  
    band3_gain_right          <= band3_gain_right_r;  
    band4_gain_right          <= band4_gain_right_r;  
    gain_all_right            <= gain_all_right_r; 
      
end behavior;


























