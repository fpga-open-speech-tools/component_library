library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

package Parallel2Serial_32bits_pkg is
	component Parallel2Serial_32bits_lpm_shiftreg_180_wxh4spi is
		port (
			clock    : in  std_logic                     := 'X';             -- clock
			load     : in  std_logic                     := 'X';             -- load
			data     : in  std_logic_vector(31 downto 0) := (others => 'X'); -- data
			shiftout : out std_logic                                         -- shiftout
		);
	end component Parallel2Serial_32bits_lpm_shiftreg_180_wxh4spi;

end Parallel2Serial_32bits_pkg;
