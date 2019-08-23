----------------------------------------------------------------------------------
-- Company:          Flat Earth Inc
-- Author/Engineer:	 Tyler Davis (based on a design by Ross Snider)
-- 
-- Create Date:    11/07/2018 
-- Design Name: 
-- Module Name: FE_Qsys_Autogen_Gainv1
-- Project Name: 
-- Target Devices: DE10
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

entity FE_Qsys_Autogen_Gainv1 is
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
end FE_Qsys_Autogen_Gainv1;

architecture behavior of FE_Qsys_Autogen_Gainv1 is

  signal gain_left_r      : std_logic_vector(31 downto 0);
  signal gain_right_r      : std_logic_vector(31 downto 0);


begin

    ------------------------------------------------------------------------
    -- Read from Registers
    ------------------------------------------------------------------------ 
    process(clk)
    begin
      if rising_edge(clk) and (avs_s1_read = '1') then  -- all registers can be read. 
        case avs_s1_address is
          when "0"  => avs_s1_readdata <= gain_left_r;
          when "1"  => avs_s1_readdata <= gain_right_r;
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
            gain_left_r     <= x"10000000";  -- W32F16
            gain_right_r     <= x"10000000";  -- W32F16
      elsif rising_edge(clk) and (avs_s1_write = '1') then  -- write the registers
        case avs_s1_address is
          when "0"  => gain_left_r             <= avs_s1_writedata;
          when "1"  => gain_right_r           <= avs_s1_writedata;
          when others  => null;                     
        end case;
      end if;    
    end process;
      
    gain_left           <= gain_left_r;
    gain_right           <= gain_right_r; 
      
end behavior;


























