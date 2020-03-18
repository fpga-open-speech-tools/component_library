library IEEE;
use IEEE.std_logic_1164.all;

package i2c is 
  type i2c_rec is record
    ena     : std_logic;
    addr    : std_logic_vector(6 downto 0);
    rw      : std_logic;
    data_wr : std_logic_vector(7 downto 0);
  end record i2c_rec;
end package;