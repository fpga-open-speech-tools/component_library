library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

-- Contains constants to initialize
package fixed_resize_pkg is

  function resize_fixed (
    arg: signed;
    old_word_len : Natural; 
    old_frac_len: Natural; 
    new_word_len: Natural; 
    new_frac_len: Natural) 
  return signed;

  function resize_fixed (
    arg : unsigned;
    old_word_len : Natural; 
    old_frac_len: Natural; 
    new_word_len: Natural; 
    new_frac_len: Natural) 
  return unsigned;
  
end package fixed_resize_pkg;
package body fixed_resize_pkg is

  function min(arg1, arg2 : natural) return natural is
  begin
    if arg1 < arg2 then return arg1;
    else return arg2;
    end if;
  end min;

  function resize_fixed (
    arg: signed;
    old_word_len : Natural; 
    old_frac_len: Natural; 
    new_word_len: Natural; 
    new_frac_len: Natural) 
  return signed is
    variable result: signed(new_word_len-1 downto 0) := (others => '0');
    constant SIGN: std_ulogic := arg(arg'left);
    constant INT_BITS : Natural := (old_word_len - old_frac_len - 1);
    constant NEW_INT_BITS : Natural := (new_word_len - new_frac_len - 1);
    constant USE_FRAC_BITS : Natural := min(old_frac_len, new_frac_len);
    variable int_result : signed(NEW_INT_BITS downto 0); -- Contains sign bit and integer portion
    variable new_frac : std_logic_vector(new_frac_len - 1 downto 0);
  begin
    -- Resize integer portion
    int_result := resize(arg(old_word_len - 1 downto (old_word_len - INT_BITS - 1)), NEW_INT_BITS + 1);
    -- Initialize fraction portion with sign bit
    new_frac := (others => SIGN);
    
    -- If there are any fractional bits in new fixed point number
    if new_frac'length >= 2 then
      -- Set upper bits of fraction portion to the number of bits that should be used from the original
      new_frac(new_frac'length - 1 downto new_frac'length - USE_FRAC_BITS) := std_logic_vector(arg(old_frac_len - 1 downto old_frac_len - USE_FRAC_BITS));
    end if;
    -- 
    result := signed(std_logic_vector(int_result) & new_frac);
    return result;
  end resize_fixed;

  function resize_fixed (
    arg : unsigned;
    old_word_len : Natural; 
    old_frac_len: Natural; 
    new_word_len: Natural; 
    new_frac_len: Natural) 
  return unsigned is
    variable result: unsigned(new_word_len-1 downto 0) := (others => '0');
    constant SIGN: std_ulogic := arg(arg'left);
    constant INT_BITS : Natural := (old_word_len - old_frac_len);
    constant NEW_INT_BITS : Natural := (new_word_len - new_frac_len);
    constant USE_FRAC_BITS : Natural := min(old_frac_len, new_frac_len);
    variable int_result : unsigned(INT_BITS - 1 downto 0);
    variable new_frac : unsigned(arg'left - INT_BITS - 1 downto 0);
  begin
    int_result := resize(arg(old_word_len - 1 downto (old_word_len - INT_BITS - 1)), NEW_INT_BITS);
    new_frac := (others => SIGN);
    if new_frac'length >= 1 then
      new_frac(new_frac'length - 1 downto new_frac'length - USE_FRAC_BITS) := arg(old_frac_len - 1 downto old_frac_len - USE_FRAC_BITS);
    end if;

    result := int_result & new_frac(new_frac'length - 1 downto 0);
    return result;
  end resize_fixed;

end package body fixed_resize_pkg;