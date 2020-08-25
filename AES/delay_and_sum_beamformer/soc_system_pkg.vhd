library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

package soc_system_pkg is
	component DSBF_dataplane_avalon is
		port (
			clk                    : in  std_logic                     := 'X';             -- clk
			reset                  : in  std_logic                     := 'X';             -- reset
			avalon_slave_address   : in  std_logic                     := 'X';             -- address
			avalon_slave_read      : in  std_logic                     := 'X';             -- read
			avalon_slave_readdata  : out std_logic_vector(31 downto 0);                    -- readdata
			avalon_slave_write     : in  std_logic                     := 'X';             -- write
			avalon_slave_writedata : in  std_logic_vector(31 downto 0) := (others => 'X'); -- writedata
			avalon_sink_valid      : in  std_logic                     := 'X';             -- valid
			avalon_sink_data       : in  std_logic_vector(31 downto 0) := (others => 'X'); -- data
			avalon_sink_channel    : in  std_logic_vector(3 downto 0)  := (others => 'X'); -- channel
			avalon_sink_error      : in  std_logic_vector(1 downto 0)  := (others => 'X'); -- error
			avalon_source_valid    : out std_logic;                                        -- valid
			avalon_source_data     : out std_logic_vector(31 downto 0);                    -- data
			avalon_source_channel  : out std_logic;                                        -- channel
			avalon_source_error    : out std_logic_vector(1 downto 0)                      -- error
		);
	end component DSBF_dataplane_avalon;

end soc_system_pkg;
