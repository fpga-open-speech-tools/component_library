----------------------------------------------------------------------------
--! @file FE_NCP5623B.vhd
--! @brief 
--! @details  
--! @author Tyler Davis
--! @date 2020
--! @copyright Copyright 2020 Flat Earth Inc
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
-- Flat Earth Inc
-- 985 Technology Blvd
-- Bozeman, MT 59718
-- support@flatearthinc.com
----------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

entity FE_NCP5623B is
  
  port (
    sys_clk               : in  std_logic                     := '0';
    reset_n               : in  std_logic                     := '0';
    
    -- Avalon streaming input
    rgb_input_data        : in std_logic_vector(15 downto 0)  := (others => '0');
    rgb_input_valid       : in std_logic                      := '0'; 
    rgb_input_error       : in std_logic_vector(1 downto 0)   := (others => '0');
   
    i2c_enable_out        : out std_logic;
    i2c_address_out       : out std_logic_vector(6 downto 0) := "0111000";
    i2c_rdwr_out          : out std_logic;
    i2c_data_write_out    : out std_logic_vector(7 downto 0) := (others => '0');
    i2c_bsy_in            : in  std_logic;
    i2c_data_read_in      : in  std_logic_vector(7 downto 0) := (others => '0');
  
    i2c_req_out           : out std_logic;
    i2c_rdy_in            : in  std_logic
  );
end entity FE_NCP5623B;

architecture rtl of FE_NCP5623B is

    
  -- I2C logic signals
  signal i2c_enable : std_logic;
  signal i2c_address : std_logic_vector(6 downto 0) := "0111001";
  -- signal i2c_address : std_logic_vector(6 downto 0) := "1110110"; -- BME
  signal i2c_rdwr : std_logic;
  signal i2c_data_write : std_logic_vector(7 downto 0) := (others => '0');
  signal i2c_bsy : std_logic;
  signal i2c_rdy : std_logic;
  signal i2c_req : std_logic;
  signal i2c_data_read : std_logic_vector(7 downto 0) := (others => '0');
  
  signal ILED_OUTPUT : std_logic_vector(2 downto 0) := "001";
  signal MAX_OUTPUT  : std_logic_vector(4 downto 0) := "11111";
   
  signal PWM1 : std_logic_vector(2 downto 0) := "010"; -- BLUE
  signal PWM2 : std_logic_vector(2 downto 0) := "011"; -- RED
  signal PWM3 : std_logic_vector(2 downto 0) := "100"; -- GREEN
  
  signal PWM1_COLOR : std_logic_vector(4 downto 0) := "00000";
  signal PWM2_COLOR : std_logic_vector(4 downto 0) := "00000";
  signal PWM3_COLOR : std_logic_vector(4 downto 0) := "00000";
   
  signal i2c_write : std_logic := '0';
  signal first_byte : std_logic_vector(7 downto 0) := (others => '0');
  signal second_byte : std_logic_vector(7 downto 0) := (others => '0');
  signal write_two : std_logic := '0';
  signal second_byte_loaded : std_logic := '0';
  
  signal init_cntr    : integer := 0;
  signal init_cycles  : integer := 5000;
    
  type i2c_state is ( idle,enable_wait,load_first_byte,load_second_byte,
                      tx_wait,i2c_busy_wait, init_device,
                      load_r, load_g, load_b); 
                      
  -- Enable recovery from illegal state
  attribute syn_encoding        : string;
  attribute syn_encoding of i2c_state : type is "safe";
  
  signal cur_i2c_state : i2c_state := init_device;
  signal next_i2c_state : i2c_state := idle;
  
begin 
  
  data_in_process : process(sys_clk,reset_n)
  begin 
    if reset_n = '0' then 
      PWM1_COLOR <= (others => '0');
      PWM2_COLOR <= (others => '0');
      PWM3_COLOR <= (others => '0');
    elsif rising_edge(sys_clk) then 
      if rgb_input_valid = '1'  then 
        PWM1_COLOR <= rgb_input_data(4 downto 0);   -- Blue
        PWM3_COLOR <= rgb_input_data(9 downto 5);   -- Green
        PWM2_COLOR <= rgb_input_data(14 downto 10); -- Red
        i2c_write  <= '1';
      else
        -- PWM1_COLOR <= PWM1_COLOR;
        -- PWM2_COLOR <= PWM2_COLOR;
        -- PWM3_COLOR <= PWM3_COLOR;  
        i2c_write  <= '0';
      end if;
    end if;
  end process;

  
  i2c_transition_process: process(sys_clk,reset_n)
  begin
    if reset_n = '0' then 
      cur_i2c_state <= init_device;
    elsif rising_edge(sys_clk) then 
      case cur_i2c_state is

        when init_device =>
          if i2c_rdy = '1' and init_cntr = init_cycles then 
            cur_i2c_state <= load_first_byte;
          else 
            cur_i2c_state <= init_device;
          end if;

        when idle => 
          -- Wait for the streaming data to be valid before writing the colors to the driver
          if i2c_write = '1' then 
            cur_i2c_state <= enable_wait;
          end if;
          
        when enable_wait => 
          -- Wait for the top level to give access to the I2C master before starting to send I2C data
          if i2c_rdy = '1' then 
            cur_i2c_state <= load_r;
          else 
            cur_i2c_state <= enable_wait;
          end if;

        when load_r =>
            cur_i2c_state <= load_first_byte;

        when load_g =>
            cur_i2c_state <= load_first_byte;

        when load_b =>
            cur_i2c_state <= load_first_byte;

        when load_first_byte =>
          cur_i2c_state <= i2c_busy_wait;

        when i2c_busy_wait =>
          if i2c_bsy = '1' and write_two = '1' and second_byte_loaded = '0' then 
            cur_i2c_state <= load_second_byte;
          elsif i2c_bsy = '1' then 
            cur_i2c_state <= tx_wait;
          else
            cur_i2c_state <= i2c_busy_wait;
          end if;

        when load_second_byte =>
          cur_i2c_state <= tx_wait;

        when tx_wait =>
          -- When the core is no longer busy and only one byte is written, move to the next state
          if i2c_bsy = '0' and write_two = '0' then 
            second_byte_loaded <= '0';
            cur_i2c_state <= next_i2c_state;
          
          -- Otherwise, if the core isn't busy but the second byte isn't loaded then send the next byte
          elsif i2c_bsy = '0' and second_byte_loaded = '0' then 
            cur_i2c_state <= i2c_busy_wait;
            second_byte_loaded <= '1';
            
          -- When the core isn't busy and the second byte has been loaded, move to the next state
          elsif i2c_bsy = '0' and second_byte_loaded = '1' then 
            cur_i2c_state <= next_i2c_state;
            second_byte_loaded <= '0';
            
          -- Otherwise, wait for the core to finish sending I2C data
          else 
            cur_i2c_state <= tx_wait;
          end if;

        when others =>

      end case;

    end if;
  end process;

  i2c_logic_process: process(sys_clk,reset_n)
  begin
    if rising_edge(sys_clk) then 
      case cur_i2c_state is

        when init_device =>
          -- Initialize the device
          first_byte <= ILED_OUTPUT & MAX_OUTPUT;
          i2c_req <= '1';
          if init_cntr < init_cycles then 
          init_cntr <= init_cntr + 1;   
          else 
            init_cntr <= init_cntr;
          end if;

        when idle => 
          -- Disable the I2C enable signal and cancel the request to use the master
          i2c_enable <= '0';
          i2c_req <= '0';
          
        when enable_wait => 
          -- Request to use the master
          i2c_req <= '1';

        when load_r =>
          -- Load the data into the I2C signals
          first_byte <= PWM2 & PWM2_COLOR;
          
          -- set the next I2C state
          next_i2c_state <= load_g;

        when load_g =>
          -- Load the data into the I2C signals
          first_byte <= PWM3 & PWM3_COLOR;
          
          -- set the next I2C state
          next_i2c_state <= load_b;

        when load_b =>
          -- Load the data into the I2C signals
          first_byte <= PWM1 & PWM1_COLOR;
          
          -- set the next I2C state
          next_i2c_state <= idle;

        when load_first_byte =>
          i2c_rdwr <= '0';
          i2c_data_write <= first_byte;
          i2c_enable <= '1';

        when load_second_byte =>
          i2c_data_write <= second_byte;

        when i2c_busy_wait =>

        when tx_wait =>
          if second_byte_loaded = '1' then 
            i2c_enable <= '0';
          elsif write_two = '0' then 
            i2c_enable <= '0';
          else
            i2c_enable <= '1';
          end if;

        when others =>

        end case;

    end if;
  end process;
  
  -- Map the I2C ready and request signals
  i2c_rdy               <= i2c_rdy_in;
  i2c_req_out           <= i2c_req;
  
  -- Map the signals for the I2C master
  i2c_address_out       <= i2c_address;
  i2c_rdwr_out          <= i2c_rdwr;
  i2c_data_write_out    <= i2c_data_write;
  i2c_enable_out        <= i2c_enable;
  
  i2c_bsy               <= i2c_bsy_in;       
  i2c_data_read         <= i2c_data_read_in;
  

end architecture rtl;























































