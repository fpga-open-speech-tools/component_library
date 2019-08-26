--!@file
----------------------------------------------------------------------------------
-- Company:          Flat Earth Inc
-- Author/Engineer:  Tyler Davis
-- 
-- Create Date:    11/1/2018
-- Design Name: 
-- Module Name:    
-- Project Name: 
-- Target Devices: DE10-Nano-SoC
-- Tool versions: 
-- Description: 
-- Copyright:   2018 Flat Earth, Inc
--
-- Dependencies: 
--
-- Revision: 
-- Revision 0.01 - File Created
-- Additional Comments: 
--
----------------------------------------------------------------------------------

--! @brief A simple left/right to multiplexed channel avalon streaming interface converter
--!
--! @details This component converts a left/right audio input to a multiplexed audio
--! channel output.  
--!

library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

entity FE_LR_To_Multichannel_Output is
    port (
        sys_clk                   : in  std_logic                     := '0';             --           sys_clk.clk
        sys_reset                 : in  std_logic                     := '0';             --         sys_reset.reset_n
        data_in_left_data         : in  std_logic_vector(31 downto 0) := (others => '0'); --      left_data_in.data
        data_in_left_error        : in  std_logic_vector(1 downto 0)  := (others => '0'); --                  .error
        data_in_left_valid        : in  std_logic                     := '0';             --                  .valid
        data_in_right_data        : in  std_logic_vector(31 downto 0) := (others => '0'); --     right_data_in.data
        data_in_right_valid       : in  std_logic                     := '0';             --                  .valid
        data_in_right_error       : in  std_logic_vector(1 downto 0)  := (others => '0'); --                  .error
        multiplex_data_out_data    : out std_logic_vector(31 downto 0);                    -- multichannel_data.data
        multiplex_data_out_error   : out std_logic_vector(1 downto 0);                     --                  .error
        multiplex_data_out_channel : out std_logic_vector(1 downto 0);                     --                  .channel
        multiplex_data_out_valid   : out std_logic                                         --                  .valid
    );
end entity FE_LR_To_Multichannel_Output;

architecture rtl of FE_LR_To_Multichannel_Output is

  -- Create signals for the outputs
	signal  data_out            :  std_logic_vector(31 downto 0) := (others => '0');
	signal	valid_out           :  std_logic := '0';
  signal	channel_out         :  std_logic_vector( 1 downto 0) := (others => '0');
	signal	error_out           :  std_logic_vector( 1 downto 0) := (others => '0'); 
    
begin
  
  process (sys_clk)
  begin
    if (rising_edge(sys_clk)) then      
      if data_in_left_valid = '1' then 
        data_out <= data_in_left_data;
        valid_out <= '1';
        error_out <= data_in_left_error;
        channel_out <= "00";
      elsif data_in_right_valid = '1' then 
        data_out <= data_in_right_data;
        valid_out <= '1';
        error_out <= data_in_right_error;
        channel_out <= "01";
      else
        valid_out <= '0';
        channel_out <= "11";
      end if; -- valid conditionals
    end if; -- rising edge conditional
  end process;
  
  -- Map the output signals to the ports
  multiplex_data_out_data    <= data_out;
  multiplex_data_out_valid   <= valid_out;
  multiplex_data_out_error   <= error_out;
  multiplex_data_out_channel <= channel_out;

end architecture rtl; -- of new_component


   






































