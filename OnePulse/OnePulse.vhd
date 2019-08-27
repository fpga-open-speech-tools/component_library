LIBRARY IEEE;
USE  IEEE.STD_LOGIC_1164.all;
USE  IEEE.STD_LOGIC_ARITH.all;
USE  IEEE.STD_LOGIC_UNSIGNED.all;

---------------------------------------------------------------------------------
-- Debounce a pushbutton input signal and output a synchronized pulse
-- Filters out mechanical switch bounce for around 40Ms.
---------------------------------------------------------------------------------
entity OnePulse is
	port(
		clk      	    			: in  std_logic;  
		reset                   : in  std_logic;
		push_button	            : in  std_logic;  -- push button to generate pulse
		single_pulse            : out std_logic   -- pulse of 1 clock cycle sent out when push button is pressed
		);
end entity;

architecture OnePulse_arch of OnePulse is

	
	type state_type is (state_wait, state_high, state_low);
	signal state   : state_type;

begin
	
	
	-- Logic to advance to the next state
	process (clk, reset)
	begin
		if reset = '1' then
			state <= state_wait;
		elsif (rising_edge(clk)) then
			case state is
				when state_wait =>
					if push_button = '1' then
						state <= state_high;
					else
						state <= state_wait;
					end if;
				when state_high =>
					state <= state_low;
				when state_low =>
					if push_button = '1' then  -- stay in low state until signal is deasserted
						state <= state_low;
					else
						state <= state_wait;
					end if;
				when others =>
					state <= state_wait;
			end case;
		end if;
	end process;

	-- Output depends solely on the current state
	process (state)
	begin
		case state is
			when state_wait =>
				single_pulse <= '0';
			when state_high =>
				single_pulse <= '1';
			when state_low =>
				single_pulse <= '0';
		end case;
	end process;
	


	
end architecture;

