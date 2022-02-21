#!/bin/sh
INTEL_SIM_LIB=/usr/local/intelFPGA/20.1/quartus/eda/sim_lib
mkdir -p work
rm ad1939.vcd
#/usr/lib/ghdl/vendors/compile-intel.sh --altera --src $INTEL_SIM_LIB &&
ghdl -i --ieee=synopsys --workdir=work -Paltera ../../serdes/Parallel2Serial_32bits.vhd ../../serdes/Serial2Parallel_32bits.vhd ../ad1939_pkg.vhd ../AD1939_hps_audio_mini.vhd ad1939_tb.vhd &&
ghdl -m -g --ieee=synopsys --workdir=work -Paltera ad1939_tb &&
ghdl -r --workdir=work ad1939_tb --vcd=ad1939.vcd &&
gtkwave ad1939.vcd &
