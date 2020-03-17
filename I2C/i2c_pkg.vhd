library IEEE;
use IEEE.std_logic_1164.all;

package i2c is 
  type i2c_rec is record
    ena     : std_logic;
    addr    : std_logic_vector(6 downto 0);
    rw      : std_logic;
    data_wr : std_logic_vector(7 downto 0);
    data_rd : std_logic_vector(7 downto 0);
    busy    : std_logic;
  end record i2c_rec;

  -- type i2c_data_rec is record
  --   busy : std_logic;
  --   data_rd : std_logic_vector(7 downto 0);
  --   ack_error : std_logic;
  -- end record i2c_data_rec;
end package;

package body i2c is
  -- --! Converts i2c control record into a std_logic_vector
  -- function std_logic_vector(i2c : i2c_rec) return std_logic_vector is
  --   variable result : std_logic_vector(16 downto 0);
  -- begin
  --   result := i2c.ena & i2c.addr & i2c.rw & i2c.data_wr;
  --   return result;
  -- end function;

  -- --! Converts i2c data record into a std_logic_vector
  -- function std_logic_vector(i2c_data : i2c_data_rec) return std_logic_vector is
  --   variable result : std_logic_vector(9 downto 0);
  -- begin
  --   result := i2c_data.busy & i2c_data.data_rd & i2c_data.ack_error;
  --   return result;
  -- end function;

end package body;