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
--
-- Dependencies: 
--
-- Revision: 
-- Revision 0.01 - File Created
-- Additional Comments: 
--
----------------------------------------------------------------------------------

--! @brief A simple multiplexed to left/right channel avalon streaming interface converter
--!
--! @details This component converts a multiplexed audio input to a left/right
--! channel output.  
--!

library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

entity FE_Multichannel_To_LR_Output is
	port (
		sys_clk                   : in  std_logic                     := '0';             --           sys_clk.clk
		sys_reset                 : in  std_logic                     := '0';             --         sys_reset.reset
		multiplex_data_in_data    : in  std_logic_vector(31 downto 0) := (others => '0'); -- multiplex_data_in.data
		multiplex_data_in_valid   : in  std_logic                     := '0';             --                  .valid
		multiplex_data_in_error   : in  std_logic_vector(1 downto 0)  := (others => '0'); --                  .error
		multiplex_data_in_channel : in  std_logic_vector(1 downto 0)  := (others => '0'); --                  .channel
		data_out_left_data            : out std_logic_vector(31 downto 0);                    --         data_left.data
		data_out_left_error           : out std_logic_vector(1 downto 0);                     --                  .error
		data_out_left_valid           : out std_logic;                                        --                  .valid
		data_out_right_data           : out std_logic_vector(31 downto 0);                    --        data_right.data
		data_out_right_error          : out std_logic_vector(1 downto 0);                     --                  .error
		data_out_right_valid          : out std_logic                                         --                  .valid
	);
end entity FE_Multichannel_To_LR_Output;

architecture rtl of FE_Multichannel_To_LR_Output is

  -- Create signals for the outputs
	signal  data_left_data_out            :  std_logic_vector(31 downto 0):= (others => '0');
	signal	data_left_error_out           :  std_logic_vector(1 downto 0):= (others => '0');
	signal	data_left_valid_out           :  std_logic := '0';  
	signal	data_right_data_out           :  std_logic_vector(31 downto 0):= (others => '0');
	signal  data_right_error_out          :  std_logic_vector(1 downto 0):= (others => '0');
	signal  data_right_valid_out          :  std_logic := '0';
  
begin

  process (sys_clk)
  begin
    if (rising_edge(sys_clk)) then      
    -- Wait for valid data
    if multiplex_data_in_valid  = '1' then
      -- When valid data has arrived, assign 
      case multiplex_data_in_channel is
        when "00" => 
          data_left_valid_out   <= multiplex_data_in_valid;
          data_left_data_out    <= multiplex_data_in_data;
          data_left_error_out   <= multiplex_data_in_error;
        when "01" => 
          data_right_valid_out  <= multiplex_data_in_valid;
          data_right_data_out   <= multiplex_data_in_data;
          data_right_error_out  <= multiplex_data_in_error;
        when others =>
      end case;
      -- When the data is not valid, set the valid bits to 0
      else
        data_left_valid_out   <= '0';
        data_right_valid_out  <= '0';
      end if; -- valid conditional
    end if; -- rising edge conditional
  end process;
  
  -- Map the output signals to the ports
  data_out_left_data    <= data_left_data_out;
  data_out_left_valid   <= data_left_valid_out;
  data_out_left_error   <= data_left_error_out;
  data_out_right_data   <= data_right_data_out;
  data_out_right_valid  <= data_right_valid_out;
  data_out_right_error  <= data_right_error_out;
  
end architecture rtl;





































