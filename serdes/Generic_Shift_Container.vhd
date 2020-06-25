----------------------------------------------------------------------------
--! @file Generic_Shift_Container.vhd
--! @brief Generic shift register component
--! @details  
--! @author Tyler Davis
--! @date 2020
--! @copyright Copyright 2020 Audio Logic
--
--  Permission is hereby granted, free of charge, to any person obtaining a copy
--  of this software and associated documentation files (the "Software"), to deal
--  IN the Software without restriction, including without limitation the rights
--  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
--  copies of the Software, and to permit persons to whom the Software is furnished
--  to do so, subject to the following conditions:
--
--  The above copyright notice and this permission notice shall be included IN all
--  copies or substantial portions of the Software.
--
--  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
--  INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
--  PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
--  HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
--  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
--  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
--
-- Tyler Davis
-- Audio Logic
-- 985 Technology Blvd
-- Bozeman, MT 59718
-- openspeech@flatearthinc.com
----------------------------------------------------------------------------
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
output_select: process(clk)
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