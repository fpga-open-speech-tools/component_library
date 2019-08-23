------------------------------------------------------------------------------
----
----! @file       spi_commands.vhd
----! @brief      SPI addressed command abstraction
----!             Send a command/address/payload onto an SPI slave. 
----!             
----! @details   
----! @copyright  
----! @author     Chris Casebeer
----! @version    
----!@date       1_13_2015
----!@copyright
----
----This program is free software : you can redistribute it and / or modify
----it under the terms of the GNU General Public License as published by
----the Free Software Foundation, either version 3 of the License, or
----(at your option) any later version.
----
----This program is distributed in the hope that it will be useful,
----but WITHOUT ANY WARRANTY; without even the implied warranty of
----MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
----GNU General Public License for more details.
----
----You should have received a copy of the GNU General Public License
----along with this program.If not, see <http://www.gnu.org/licenses/>.
----
----Chris Casebeer
----Electrical and Computer Engineering
----Montana State University
----610 Cobleigh Hall
----Bozeman, MT 59717
----christopher.casebee1@msu.montana.edu
----
------------------------------------------------------------------------------
------------------------------------------------------------------------------

------------------------------------------------------------------------------
--
--! @brief      An SPI addressed command abstraction
--! @details    
--!   
--! @param      command_used_g        Use a command with the abstraction. 
--! @param      address_used_g        Use an address with the abstraction.  
--! @param      command_width_bits_g       The width of the command in bytes. Must be >=1;  
--! @param      address_width_bits_g       The width of the address in bytes. Must be >=1;  
--!                                         If no address is used, this must be set to 0.
--! @param      data_length_bit_width_g     The bit width of the data_length_in
--!                                         port. Allows greater payload lengths.      
--! @param      cpol_cpha             Target a 00 or 11 implementation of SPI. Default is 00.    
--!
--! @param      clk                   System clock which drives entity. This is also your SPI clock speed.
--! @param      rst_n                 Active Low reset to reset entity 
--! @param      command_in            Command to send to spi slave.
--! @param      address_in            Address to send after command if addressed. 
--! @param      data_length_in        The length of the payload in bytes(write/read)
--!                                   which will come after command/address
--!                                      
--! @param      master_slave_data_in            New payload byte to send over SPI.
--!                                             This data is sampled by the entity on
--!                                             master_slave_data_rdy_in. A new piece of data
--!                                             can only be accepted by the entity upon 
--!                                             master_slave_data_ack_out going high.
--! @param      master_slave_data_rdy_in        Indicate that a new payload byte is present 
--!                                             on master_slave_data_in. That byte will 
--!                                             then be sent to the slave.
--!                                            
--!                                                 
--! @param      master_slave_data_ack_out   The entity can accept another payload byte upon
--!                                         this line going high. User should then load and
--!                                         signal a new payload byte with master_slave_data_rdy_in. 
--!                                         This line is map of spi_abstract's mosi_data_ack_o.
--!                                         However, due to the command nature of spi_command, 
--!                                         master_slave_data_ack_out only goes high (more data accepted)
--!                                         after the command/address/first data byte have been moved. 
--! @param      command_busy_out            The spi command abstraction is busy
--!                                         servicing a command. Do not attempt
--!                                         to send another command. If this is '0'
--!                                         the entity is ready to send another command.
--! @param      command_done                Command done pulse.
--!
--! @param      slave_master_data_out       The MISO byte is valid  
--! @param      slave_master_data_ack_out   A payload associated byte has been 
--!                                         received back from the slave. Sample
--!                                         slave_master_data_out now.    
--!                                         This is a relay of the spi_abstract signal 
--!                                         miso_data_valid_o. However in this implementation
--!                                         it is only relayed upon completion of command/address
--!                                         portion of the SPI data set. 
--!                                         
--! @param      miso       MISO line from device  
--! @param      mosi       MOSI line to device   
--! @param      sclk       SCLK line to device   
--! @param      cs_n       CS_N line to device   
--
------------------------------------------------------------------------------

-- Usage instructions and Description:

-- SPI is a interchip device bus which is used to send data back and forth
-- between two devices. Data is synchronous to a clock (sclk) which the 
-- master supplies to the slave. Data is sent from the master to the slave
-- on the MOSI line. Data is sent back to the master on the MISO line. 
-- All data is valid on the rising edge of sclk. The chip select line
-- can be used to select specific slaves given a shared bus. In a single slave
-- scenario is simply starts or ends a communication.

-- A command is sent to the spi slave by loading 
-- command_in,address_in,data_length_in,master_slave_data_in and then
-- pulsing master_slave_data_rdy_in. 
-- The abstraction will process this amount of data over the SPI bus. It will
-- then drive master_slave_data_ack_out high to signal that another 
-- byte of the payload should be sent to the entity. This is when the
-- user of the entity should update master_slave_data_in and pulse 
-- master_slave_data_rdy_in. The total number of master_slave_data_rdy_in's pulses 
-- sent to the entity should match exactly the data_length_in on the first master_slave_data_rdy.
-- This is because the first byte of payload is associated with the first master_slave_data_rdy

-- For every data section byte sent to the slave device, 
-- a byte is received back from the SPI slave over the MISO line. 
-- This byte is loaded into slave_master_data_out and indicated
-- valid by a pulse on slave_master_data_ack_out. 
-- The data should be taken at this time if of interest. 

--If a command or address are to be omitted such as in the case
--of the SPI bus functioning as a register transfer between devices, 
--the generics command_used_g and address_used_g can be set to 0. This
--will make the abstraction ignore the command/address ports and send
--the first data byte and subsequent payload bytes. The first byte
--sent to the slave in this instance will have an associated slave_master_data_ack_out.

--In the instance of addressed commands, the first slave_master_data_ack_out
--signal will be associated with the first data byte of payload sent out on MOSI. 



--Received bytes are only signalled ready at slave_master_data_ack_out
--that are associated with the sent data portion bytes of the master.
--Thus bytes received back by master when the master is sending out
--command or address sections of the payload are not relayed up. This is
--because in most instances these bytes are not used by the host.

--slave_master_data_ack_out is a conditional map of spi_abstract's,
--miso_data_valid_o. 

--TODO:

--Allow command to be turned off and on without generic. 
--Or simply just remove generics all together and rely on ports. 

--Allow a pathway for address without a command. 





library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;



entity spi_commands is
  generic(
  command_used_g        	: std_logic := '0';
  address_used_g        	: std_logic := '0';
  command_width_bits_g 	  : natural 	:= 1;
  address_width_bits_g 	  : natural 	:= 1;
  data_width_bits_g 	    : natural 	:= 8;
  output_bits_g           : natural   := 10;
  cpol_cpha             	: std_logic_vector(1 downto 0) := "00"
);
	port(
      clk	            :in	std_logic;	
		  rst_n 	        :in	std_logic;
      
      command_in      : in  std_logic_vector(command_width_bits_g-1 downto 0);
      address_in      : in  std_logic_vector(address_width_bits_g-1 downto 0);
      
      master_slave_data_in      :in   std_logic_vector(data_width_bits_g-1 downto 0);
      master_slave_data_rdy_in  :in   std_logic;
      master_slave_data_ack_out :out  std_logic;
      command_busy_out          :out  std_logic;
      command_done              :out  std_logic;

      slave_master_data_out     : out std_logic_vector(output_bits_g-1 downto 0);
      slave_master_data_ack_out : out std_logic;

      miso 				:in	  std_logic;	
      mosi 				:out  std_logic;	
      sclk 				:out  std_logic;	
      cs_n 				:out  std_logic
		 
		);
end spi_commands;

architecture Behavioral of spi_commands is

function to_natural( sl_in : std_logic ) return natural is
begin
      if sl_in = '1' then
      return 1;
   else
      return 0;
   end if;
end function;

component spi_abstract is
 generic (
      cpol_cpha : std_logic_vector(1 downto 0) := "00";
      data_width: natural := output_bits_g
 );
	port(
      clk	  :in	std_logic;	
		  rst_n 	    :in	std_logic;	

      mosi_data_i         : in std_logic_vector(data_width-1 downto 0);
      miso_data_o         : out std_logic_vector(data_width-1 downto 0);
      mosi_data_valid_i 	:in	std_logic;	
      mosi_data_ack_o 	  :out	std_logic;	
      miso_data_valid_o 	:out	std_logic;	
      
		  miso 				:in	std_logic;	
      mosi 				:out  std_logic;	
      sclk 				:out  std_logic;	
      cs_n 				:out  std_logic
		 
		);
end component;
						

    type SPI_STATE is   (
    SPI_STATE_WAIT,
    SPI_STATE_PAYLOAD,
    SPI_STATE_COMMAND_DONE,
    SPI_STATE_COMMAND_DONE_SIGNAL
    );
    
    
    
  signal cur_spi_state  : SPI_STATE;

  --Registers that store the data presented to the entity on the first
  --master_slave_data_rdy_in
  
  constant xfer_len : natural   :=  to_natural(command_used_g) * command_width_bits_g + 
                                    to_natural(address_used_g) * address_width_bits_g + 
                                    data_width_bits_g;
                    
  signal command_signal : std_logic_vector(command_width_bits_g-1 downto 0);
  signal address_signal : std_logic_vector(address_width_bits_g-1 downto 0);

  signal data_signal :std_logic_vector(data_width_bits_g-1 downto 0);  

  --Signals used to interact with the spi_abstract entity.

  signal mosi_data_valid_spi : std_logic;
  signal mosi_data_ack_spi : std_logic;
  signal mosi_data_ack_spi_follower : std_logic;
  signal miso_data_valid_spi : std_logic;
  --Do not assign reset signals to signals which serve only as
  --mapping. They should be reset only at the most base entity. 
  
  signal miso_data_spi : std_logic_vector(xfer_len-1 downto 0);
  signal mosi_data_spi : std_logic_vector(xfer_len-1 downto 0);
    
  --Signals used to only relay slave_master_data_ack's related to the 
  --data payload.
  signal miso_byte_ack_count : unsigned (7 downto 0);
  signal slave_master_data_ack_out_en : std_logic;
  
  --Readable cs_n.
  signal cs_n_signal : std_logic;
  signal cs_n_signal_follower : std_logic;
  
  signal data_select : std_logic_vector(1 downto 0) := command_used_g & address_used_g;
 
  
begin
  slave_master_data_out <= miso_data_spi;
  cs_n <= cs_n_signal;

  
  spi_slave: spi_abstract
  generic map(
      cpol_cpha => cpol_cpha,
      data_width => output_bits_g
  )
	port map(
      clk	    => clk,
		  rst_n 	      => rst_n,

      mosi_data_i    =>  mosi_data_spi,
      miso_data_o    => miso_data_spi,
      mosi_data_valid_i  => mosi_data_valid_spi,
      mosi_data_ack_o 	  => mosi_data_ack_spi,
      miso_data_valid_o  => miso_data_valid_spi,

		  miso 				=> miso,
      mosi 				=> mosi,
      sclk 				=> sclk,
      cs_n 				=> cs_n_signal
		 
		);

----------------------------------------------------------------------------
--
--! @brief    Send a command/address/data to the SPI slave using SPI_ABSTRACT
--!          
--! @details  This state machine effectively samples the input lines on a rdy_in
--!           pulse. It then sends this data to spi_abstract, waiting for rdy for
--!           more data signals appropriately. The state machine also decides if
--!           command/address are present and behaves accordingly. The state machine
--!           also tracks the number of payload bytes sent, so as to stop spi_abstract
--!           appropriately. 
--!           
--
--! @param    clk             Take action on positive edge.
--! @param    rst_n           rst_n to initial state.
--
----------------------------------------------------------------------------

-- State transition state machine
spi_state_state_machine: process(clk, rst_n)
begin 
  if rst_n = '0' then 
    cur_spi_state <= SPI_STATE_WAIT;
    
  elsif rising_edge(clk) then 
    case cur_spi_state is 
      when SPI_STATE_WAIT => 
      if (master_slave_data_rdy_in = '1') then
          cur_spi_state <= SPI_STATE_PAYLOAD;
      else
        cur_spi_state <= SPI_STATE_WAIT;
      end if;    

      when SPI_STATE_PAYLOAD =>
        cur_spi_state <= SPI_STATE_COMMAND_DONE;

      when SPI_STATE_COMMAND_DONE   =>      
        if (cs_n_signal_follower /= cs_n_signal) and (cs_n_signal = '1') then
            cur_spi_state <= SPI_STATE_COMMAND_DONE_SIGNAL;
        else
          cur_spi_state <= SPI_STATE_COMMAND_DONE;
        end if; 

      when SPI_STATE_COMMAND_DONE_SIGNAL =>
        cur_spi_state <= SPI_STATE_WAIT;
      when others =>
    end case;
  end if;
end process spi_state_state_machine;
  
spi_command_state_machine:  process (clk, rst_n)
begin
  if rst_n = '0' then

    command_signal <= (others => '0');
    address_signal <= (others => '0');
    data_signal <= (others => '0');
    mosi_data_valid_spi <= '0';
    mosi_data_spi <= (others => '0');
    mosi_data_ack_spi_follower <= '0';
    master_slave_data_ack_out <= '0';
    cs_n_signal_follower <= '1';
     
  elsif rising_edge(clk) then

    mosi_data_valid_spi <= '0';
    master_slave_data_ack_out <= '0';
  
    case cur_spi_state is
    
    when SPI_STATE_WAIT =>
      --Reset this follower to detect the first instance of a 
      --data_i_ack positive level.
      mosi_data_ack_spi_follower <= '0';
      if (master_slave_data_rdy_in = '1') then
        command_signal <= command_in;
        address_signal <= address_in;
        data_signal <= master_slave_data_in;
      end if;
      
    when SPI_STATE_PAYLOAD   =>  
      if (mosi_data_ack_spi_follower /= mosi_data_ack_spi) then
        mosi_data_ack_spi_follower <= mosi_data_ack_spi;
        
        case data_select is 
          when "00" =>
            mosi_data_spi <= data_signal;
            
          when "01" =>
            mosi_data_spi <= address_signal & data_signal;
            
          when "11" =>
            mosi_data_spi <= command_signal & address_signal & data_signal;
          
          when "10" =>
            mosi_data_spi <= command_signal & data_signal;
        end case;
        
        mosi_data_valid_spi <= '1';
      else
          mosi_data_valid_spi <= '0';
      end if;
    
      --Allow csn to come back up after a full command
      --CSN comes back after spi_abstract is left without any new data.
      --This is necessary between commands. 
      when SPI_STATE_COMMAND_DONE   =>      
        if (cs_n_signal_follower /= cs_n_signal) then
          cs_n_signal_follower <= cs_n_signal;
        end if; 
        
      when SPI_STATE_COMMAND_DONE_SIGNAL   =>      
        
      end case ;
  end if ;
end process spi_command_state_machine ;

----------------------------------------------------------------------------
--
--! @brief    Output logic of the main state machine.
--!          
--! @details  As of right now, command_busy is associated with every state, 
--!           except wait state.
--!           
--
--! @param    clk             Take action on positive edge.
--! @param    rst_n           rst_n to initial state.
--
----------------------------------------------------------------------------

spi_state_output:  process (cur_spi_state)
begin

--Default values
command_busy_out <= '1';
command_done <= '0';
  
case cur_spi_state is

  when SPI_STATE_WAIT =>
    command_busy_out <= '0';
  when SPI_STATE_PAYLOAD =>
  when SPI_STATE_COMMAND_DONE => 
  when SPI_STATE_COMMAND_DONE_SIGNAL =>
    command_done <= '1';

 
end case;

end process spi_state_output ;

----------------------------------------------------------------------------
--
--! @brief    Associate miso_data_acks with payload portion of command stream.
--!          
--! @details  This process is responsible for only passing the slave_master_data_ack_out's
--!           which are associated with the payload/data bytes of the command stream. In general the data received
--!           back on miso during command/address send out are not used by the host.
--!           
--
--! @param    clk             Take action on positive edge.
--! @param    rst_n           rst_n to initial state.
--
----------------------------------------------------------------------------


slave_master_data_out_handler : process(clk,rst_n)
begin
if rst_n = '0' then
  miso_byte_ack_count <= to_unsigned(0,miso_byte_ack_count'length);
  slave_master_data_ack_out_en <= '0';
  slave_master_data_ack_out <= '0';
elsif rising_edge(clk) then

    if (cur_spi_state = SPI_STATE_WAIT) then
    miso_byte_ack_count <= to_unsigned(0,miso_byte_ack_count'length);
    slave_master_data_ack_out_en <= '0';
    elsif ( miso_data_valid_spi = '1') then
    miso_byte_ack_count <= miso_byte_ack_count + 1;
    end if;
  --Attempting to get around >=.
  --Will send the next miso_data_valid_spi. 

  if(command_used_g = '1' and address_used_g = '1') then
    if ( miso_byte_ack_count = to_unsigned(command_width_bits_g + address_width_bits_g,miso_byte_ack_count'length)) then
      slave_master_data_ack_out_en <= '1';
    end if;
  elsif(command_used_g = '1' and address_used_g = '0') then
   if ( miso_byte_ack_count = to_unsigned(command_width_bits_g,miso_byte_ack_count'length)) then
      slave_master_data_ack_out_en <= '1';
    end if;
  end if;
  
  --Strictly speaking there isn't a pathway for an address used 
  --without a command. 
  if(command_used_g = '0' and address_used_g = '0') then
    slave_master_data_ack_out_en <= '1';
  end if;
  
  if (slave_master_data_ack_out_en = '1') then
  slave_master_data_ack_out <= miso_data_valid_spi; 
  end if;

   
end if;
end process slave_master_data_out_handler;


	
end Behavioral;