library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity SG_dataplane_avalon is
  port (
    clk                       : in  std_logic;
    reset                     : in  std_logic;
    avalon_sink_valid         : in  std_logic; --boolean
    avalon_sink_data          : in  std_logic_vector(31  downto 0); --sfix32_En28
    avalon_sink_channel       : in  std_logic_vector(1   downto 0); --ufix2
    avalon_sink_error         : in  std_logic_vector(1   downto 0); --ufix2
    avalon_source_valid       : out std_logic; --boolean
    avalon_source_data        : out std_logic_vector(31  downto 0); --sfix32_En28
    avalon_source_channel     : out std_logic_vector(1   downto 0); --ufix2
    avalon_source_error       : out std_logic_vector(1   downto 0); --ufix2
    avalon_slave_address      : in  std_logic_vector(0   downto 0);            
    avalon_slave_read         : in  std_logic;
    avalon_slave_readdata     : out std_logic_vector(31  downto 0);
    avalon_slave_write        : in  std_logic;
    avalon_slave_writedata    : in  std_logic_vector(31  downto 0)
  );
end entity SG_dataplane_avalon;

architecture SG_dataplane_avalon_arch of SG_dataplane_avalon is

  signal left_gain                 : std_logic_vector(31  downto 0) :=  "00000100110011001100110011001101"; -- 0.3 (sfix32_En28)
  signal right_gain                : std_logic_vector(31  downto 0) :=  "00000100110011001100110011001101"; -- 0.3 (sfix32_En28)

component SG_dataplane
  port(
    clk                         : in  std_logic; -- clk_freq = 1 Hz, period = 0.1
    reset                       : in  std_logic;
    clk_enable                  : in  std_logic;
    avalon_sink_valid           : in  std_logic;                              -- boolean
    avalon_sink_data            : in  std_logic_vector(31  downto 0);         -- sfix32_En28
    avalon_sink_channel         : in  std_logic_vector(1   downto 0);         -- ufix2
    avalon_sink_error           : in  std_logic_vector(1   downto 0);         -- ufix2
    register_control_left_gain  : in  std_logic_vector(31  downto 0);         -- sfix32_En28
    register_control_right_gain : in  std_logic_vector(31  downto 0);         -- sfix32_En28
    ce_out                      : out std_logic;
    avalon_source_valid         : out std_logic;                              -- boolean
    avalon_source_data          : out std_logic_vector(31  downto 0);         -- sfix32_En28
    avalon_source_channel       : out std_logic_vector(1   downto 0);         -- ufix2
    avalon_source_error         : out std_logic_vector(1   downto 0)          -- ufix2
  );
end component;

begin

u_SG_dataplane : SG_dataplane
  port map(
    clk                         =>  clk,
    reset                       =>  reset,
    clk_enable                  =>  '1',
    avalon_sink_valid           =>  avalon_sink_valid,               -- boolean
    avalon_sink_data            =>  avalon_sink_data,                -- sfix32_En28
    avalon_sink_channel         =>  avalon_sink_channel,             -- ufix2
    avalon_sink_error           =>  avalon_sink_error,               -- ufix2
    register_control_left_gain  =>  left_gain,                       -- sfix32_En28
    register_control_right_gain =>  right_gain,                      -- sfix32_En28
    avalon_source_valid         =>  avalon_source_valid,             -- boolean
    avalon_source_data          =>  avalon_source_data,              -- sfix32_En28
    avalon_source_channel       =>  avalon_source_channel,           -- ufix2
    avalon_source_error         =>  avalon_source_error              -- ufix2
  );

  bus_read : process(clk)
  begin
    if rising_edge(clk) and avalon_slave_read = '1' then
      case avalon_slave_address is
        when "0" => avalon_slave_readdata <= left_gain;
        when "1" => avalon_slave_readdata <= right_gain;
        when others => avalon_slave_readdata <= (others => '0');
      end case;
    end if;
  end process;

  bus_write : process(clk, reset)
  begin
    if reset = '1' then
      left_gain                 <=  "00000100110011001100110011001101"; -- 0.3 (sfix32_En28)
      right_gain                <=  "00000100110011001100110011001101"; -- 0.3 (sfix32_En28)
    elsif rising_edge(clk) and avalon_slave_write = '1' then
      case avalon_slave_address is
        when "0" => left_gain <= avalon_slave_writedata(31 downto 0);
        when "1" => right_gain <= avalon_slave_writedata(31 downto 0);
        when others => null;
      end case;
    end if;
  end process;

end architecture;