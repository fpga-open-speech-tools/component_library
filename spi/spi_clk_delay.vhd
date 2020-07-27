--!@file
----------------------------------------------------------------------------
-- Company:          Audio Logic
-- Author/Engineer:  Tyler Davis
-- 
-- Create Date:    5/20/2019
-- Design Name: 
-- Module Name:    
-- Project Name: 
-- Target Devices: DE10-Nano-SoC
-- Tool versions: 
-- Description: 
-- Copyright:   2019 Audio Logic
--
-- Dependencies: 
--
-- Revision: 
-- Revision 0.01 - File Created
-- Additional Comments: 
--
----------------------------------------------------------------------------

--! @brief This adapter converts SPI from CPHA 01 to CPHA 00
--!
--! @details This adapter converts SPI from CPHA 01 to CPHA 00 
--!

library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

entity spi_clk_delay is
    port (
        spi_clk                   : in  std_logic;
        double_spi_clk            : in  std_logic;
        sys_reset                 : in  std_logic;
        sclk_out                  : out std_logic
    );
end entity spi_clk_delay;

architecture rtl of spi_clk_delay is

  -- Create signals for the outputs
  signal sclk_delay_out       : std_logic := '0';
    
begin
  
  process (spi_clk,double_spi_clk,sys_reset)
  begin
    if falling_edge(double_spi_clk) then 
      sclk_delay_out <= spi_clk;
    end if;
  end process;
  
  -- Map the output signals to the ports
  sclk_out <= sclk_delay_out;

end architecture rtl; -- of new_component


   






































