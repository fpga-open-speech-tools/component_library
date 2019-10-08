-- Speaker_Array_Encoder.vhd

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

entity Speaker_Array_Encoder is
    port (
        sys_clk            : in  std_logic                     := '0';             --        sys_clk.clk
        reset_n            : in  std_logic                     := '0';             --      sys_reset.reset_n
        
        data_input_channel : in  std_logic_vector(3 downto 0)  := (others => '0'); --     data_input.channel
        data_input_data    : in  std_logic_vector(31 downto 0) := (others => '0'); --               .data
        data_input_error   : in  std_logic_vector(1 downto 0)  := (others => '0'); --               .error
        data_input_valid   : in  std_logic                     := '0';             --               .valid
        
        serial_data_out    : out std_logic;                                        --    serial_data.serial_data_out
        serial_control     : out std_logic;                                        -- serial_control.serial_control
        clk_out            : out std_logic                                         --     packet_clk.clk_out
    );
end entity Speaker_Array_Encoder;

architecture rtl of Speaker_Array_Encoder is
begin


    serial_data_out <= '0';

    serial_control <= '0';

    clk_out <= '0';

end architecture rtl;
