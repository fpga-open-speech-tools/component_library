----------------------------------------------------------------------------------
-- Company:          Flat Earth Inc
-- Author/Engineer:	 Chris Casebeer
-- 
-- Create Date:    6/12/2017 
-- Design Name: 
-- Module Name:    Hearing Aide Gain Qsys Block. 
-- Project Name: 
-- Target Devices: DE0-Nano-SoC
-- Tool versions: 
-- Description: 
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



entity FE_Qsys_HA_Gain_Control is
	port (
		clk 			    	: in std_logic;   
		reset_n 		    	: in std_logic;
        
        band_1_gain             : out std_logic_vector(31 downto 0);
        band_2_gain             : out std_logic_vector(31 downto 0);
        band_3_gain             : out std_logic_vector(31 downto 0);
        band_4_gain             : out std_logic_vector(31 downto 0);
        band_5_gain             : out std_logic_vector(31 downto 0);
                
        ------------------------------------------------------------
        -- Avalon Memory Mapped Slave Signals
        ------------------------------------------------------------
        avs_s1_address 	        : in  std_logic_vector( 2 downto 0);   --! Avalon MM Slave address
        avs_s1_write 		    : in  std_logic;                       --! Avalon MM Slave write
        avs_s1_writedata 	    : in  std_logic_vector(31 downto 0);   --! Avalon MM Slave write data
        avs_s1_read 		    : in  std_logic;                       --! Avalon MM Slave read
        avs_s1_readdata 	    : out std_logic_vector(31 downto 0)    --! Avalon MM Slave read data

	);
end FE_Qsys_HA_Gain_Control;

architecture behavior of FE_Qsys_HA_Gain_Control is

    signal band_1_gain_int    :  std_logic_vector(31 downto 0);
    signal band_2_gain_int    :  std_logic_vector(31 downto 0);
    signal band_3_gain_int    :  std_logic_vector(31 downto 0);
    signal band_4_gain_int    :  std_logic_vector(31 downto 0);
    signal band_5_gain_int    :  std_logic_vector(31 downto 0);

begin

    process(clk)
	begin
		if rising_edge(clk) and (avs_s1_read = '1') then  -- all registers can be read. 
			case avs_s1_address is
                when "000"  => avs_s1_readdata <= band_1_gain_int;
				when "001"  => avs_s1_readdata <= band_2_gain_int;
                when "010"  => avs_s1_readdata <= band_3_gain_int;
                when "011"  => avs_s1_readdata <= band_4_gain_int;
                when "100"  => avs_s1_readdata <= band_5_gain_int;
				when others => avs_s1_readdata <= (others => '0');
            end case;
		end if;
	end process;

    process(clk)
	begin
        if (reset_n = '0') then
            band_1_gain_int   <= x"00010000";  -- W32F16
            band_2_gain_int   <= x"00010000";  -- W32F16
            band_3_gain_int   <= x"00010000";  -- W32F16
            band_4_gain_int   <= x"00010000";  -- W32F16
            band_5_gain_int   <= x"00010000";  -- W32F16
		elsif rising_edge(clk) and (avs_s1_write = '1') then  -- write the registers
            case avs_s1_address is
                when "000"  => band_1_gain_int <= avs_s1_writedata;
                when "001"  => band_2_gain_int <= avs_s1_writedata;
                when "010"  => band_3_gain_int <= avs_s1_writedata;
                when "011"  => band_4_gain_int <= avs_s1_writedata;
                when "100"  => band_5_gain_int <= avs_s1_writedata;
                when others  => null ;
            end case;
        end if;    
	end process;
      
    band_1_gain <= band_1_gain_int;
    band_2_gain <= band_2_gain_int;
    band_3_gain <= band_3_gain_int;
    band_4_gain <= band_4_gain_int;
    band_5_gain <= band_5_gain_int;
      

end behavior;

