library ieee;
use ieee.std_logic_1164.all;

entity mono2stereo_adapter_tb is
end mono2stereo_adapter_tb;

architecture mono2stereo_adapter_tb_arch of mono2stereo_adapter_tb is

  component mono2stereo_adapter
    generic (
      sample_rate : integer := 48000;
      clk_freq    : integer := 24576000;
      data_width  : integer := 24
    );
    port (
      clk         : in  std_logic;
      rst         : in  std_logic;
      data_in     : in  std_logic_vector(data_width-1 downto 0);
      channel_in  : in  std_logic;
      valid_in    : in  std_logic;
      data_out    : out std_logic_vector(data_width-1 downto 0) := (others => '0');
      channel_out : out std_logic;
      valid_out   : out std_logic := '0'
    );
  end component;


  constant sample_rate : integer := 48000;
  constant clk_freq    : integer := 24576000;
  constant data_width  : integer := 24;
  constant cycles_per_sample : integer := clk_freq/sample_rate;

  type slv_array is array (3 downto 0) of std_logic_vector(data_width-1 downto 0);
  constant data_stim : slv_array := (x"ACBDEF", x"012345", x"A5FE3B", x"92733E");
  signal clk          : std_logic := '1';
  signal rst          : std_logic;
  signal data_in  : std_logic_vector(data_width-1 downto 0);
  signal channel_in   : std_logic;
  signal valid_in     : std_logic;
  signal data_out     : std_logic_vector(data_width-1 downto 0);
  signal channel_out  : std_logic;
  signal valid_out    : std_logic;

begin

  mono2stereo_adapter_0: mono2stereo_adapter
    port map (
      clk         => clk,
      rst         => rst,
      data_in     => data_in,
      channel_in  => channel_in,
      valid_in    => valid_in,
      data_out    => data_out,
      channel_out => channel_out,
      valid_out   => valid_out
    );

  rst <= '0';
  channel_in <= '0';

  clk_stim : process
  begin
    for i in 0 to (data_stim'length)*cycles_per_sample-1 loop
      clk <= '1';
      wait for 5 ns;
      clk <= '0';
      wait for 5 ns;
    end loop;
    wait;
  end process;

  valid_stim : process
  begin
    for i in 0 to data_stim'length-1 loop
      valid_in <= '1';
      wait for 10 ns;
      for j in 0 to cycles_per_sample-2 loop
        valid_in <='0';
        wait for 10 ns;
      end loop;
    end loop;
    wait;
  end process;

  d_stim : process
  begin
      data_in <= data_stim(0);
      wait for cycles_per_sample*10 ns;
      data_in <= data_stim(1);
      wait for cycles_per_sample*10 ns;
      data_in <= data_stim(2);
      wait for cycles_per_sample*10 ns;
      data_in <= data_stim(3);
      wait for cycles_per_sample*10 ns;
      wait;
  end process;




end architecture;
