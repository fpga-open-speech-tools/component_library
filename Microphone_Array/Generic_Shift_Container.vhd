

library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

entity Generic_Shift_Container is 
generic (
  data_width : integer := 8
);
port(
  clk         : in  std_logic;
  input_data  : in  std_logic_vector(data_width-1 downto 0);
  output_data : out std_logic_vector(data_width-1 downto 0);
  load        : in  std_logic := '0'
);
end entity Generic_Shift_Container;

architecture rtl of Generic_Shift_Container is 

signal output_data_r : std_logic_vector(data_width-1 downto 0) := (others => '0');

begin 
output_selet: process(clk)
begin

  if rising_edge(clk) then 
    if load = '1' then 
      output_data_r <= input_data;
    else
      output_data_r <= output_data_r(data_width-2 downto 0) & '0';
    end if;
  end if;
end process;

output_data <= output_data_r;

end architecture rtl;