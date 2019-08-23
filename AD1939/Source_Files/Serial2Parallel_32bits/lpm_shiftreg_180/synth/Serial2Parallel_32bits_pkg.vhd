library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

package Serial2Parallel_32bits_pkg is
	component Serial2Parallel_32bits_lpm_shiftreg_180_gizevoi is
		port (
			clock   : in  std_logic                     := 'X'; -- clock
			shiftin : in  std_logic                     := 'X'; -- shiftin
			q       : out std_logic_vector(31 downto 0)         -- q
		);
	end component Serial2Parallel_32bits_lpm_shiftreg_180_gizevoi;

end Serial2Parallel_32bits_pkg;
