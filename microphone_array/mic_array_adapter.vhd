library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

entity mic_array_adapter is
    port (
        sys_clk             : in  std_logic                     := '0';
        reset_n             : in  std_logic                     := '0';
        
        data_input_channel  : in  std_logic_vector(3 downto 0)  := (others => '0');
        data_input_data     : in  std_logic_vector(31 downto 0) := (others => '0');
        data_input_error    : in  std_logic_vector(1 downto 0)  := (others => '0');
        data_input_valid    : in  std_logic                     := '0';        
        
        data_output_channel  : out  std_logic_vector(3 downto 0)  := (others => '0');
        data_output_data     : out  std_logic_vector(31 downto 0) := (others => '0');
        data_output_error    : out  std_logic_vector(1 downto 0)  := (others => '0');
        data_output_valid    : out  std_logic                     := '0'
    );
    
end entity mic_array_adapter;

architecture rtl of mic_array_adapter is
 
signal data_r         : std_logic_vector(31 downto 0) := (others => '0');
signal channel_r      : std_logic_vector(3 downto 0) := (others => '0');
signal valid_r        : std_logic := '0';


begin 

data_process : process(sys_clk,reset_n)
begin
  if reset_n = '0' then 
    valid_r <= '0';
  elsif rising_edge(sys_clk) then 
    if data_input_valid = '1' then 
      if data_input_data(23) = '1' then 
        data_r <= "1111" & data_input_data(23 downto 0) & "0000";
      else
        data_r <= "0000" & data_input_data(23 downto 0) & "0000";
      end if;
        channel_r <= data_input_channel;
        valid_r <= '1';
    else
      valid_r <= '0';
    end if;
  end if;
end process;

-- Map the registers to the ports
data_output_data      <= data_r;
data_output_channel   <= channel_r;
data_output_valid     <= valid_r;

end architecture rtl;























































