-- FE_Signal_Energy_Monitor.vhd

-- This file was auto-generated as a prototype implementation of a module
-- created in component editor.  It ties off all outputs to ground and
-- ignores all inputs.  It needs to be edited to make it do something
-- useful.
-- 
-- This file will not be automatically regenerated.  You should check it in
-- to your version control system if you want to keep it.

library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

entity FE_Signal_Energy_Monitor is
	port (
		clk_in       : in  std_logic                     := '0';             --         clk_in.clk_in
		s1_address   : in  std_logic_vector(1 downto 0) := (others => '0'); --          s1.address
		s1_read      : in  std_logic                     := '0';             --            .read
		s1_readdata  : out std_logic_vector(31 downto 0);                    --            .readdata
		s1_write     : in  std_logic                     := '0';             --            .write
		s1_writedata : in  std_logic_vector(31 downto 0) := (others => '0'); --            .writedata
		dummy_out    : in  std_logic                     := '0';             -- conduit_end.dummy_out
		dummy_in     : in  std_logic                     := '0';             --            .dummy_in
		reset_n      : in  std_logic                     := '0'              --       reset.reset_n
	);
end entity FE_Signal_Energy_Monitor;

architecture rtl of FE_Signal_Energy_Monitor is

  -- Declare the register signals (
  signal max_reset_time_ms        :  std_logic_vector(31 downto 0);
  signal LED_persistence_time     :  std_logic_vector(31 downto 0);
  signal Fs_sample_rate_Hz        :  std_logic_vector(31 downto 0);
    
begin

	process(clk_in)
	begin
		if rising_edge(clk_in) and (s1_read = '1') then  -- all registers can be read. 
			case s1_address is
        when "00"   => s1_readdata <= max_reset_time_ms;
        when "01"   => s1_readdata <= LED_persistence_time;
        when "10"   => s1_readdata <= Fs_sample_rate_Hz;
				when others => s1_readdata <= (others => '0');
            end case;
		end if;
	end process;

    process(clk_in)
	begin
        if (reset_n = '0') then
          max_reset_time_ms       <= x"00000000";  
          LED_persistence_time    <= x"00000000";  
          Fs_sample_rate_Hz       <= x"00000000";  
		elsif rising_edge(clk_in) and (s1_write = '1') then  -- write the registers
      case s1_address is
        when "00"  => max_reset_time_ms     <= s1_writedata;
        when "01"  => LED_persistence_time  <= s1_writedata;
        when "10"  => Fs_sample_rate_Hz     <= s1_writedata;
        when others  => null ;  
      end case;
    end if;    
	end process;

end architecture rtl; -- of FE_Signal_Energy_Monitor
