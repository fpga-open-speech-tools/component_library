#!/bin/sh
mkdir -p work
ghdl -i --workdir=work *.vhd ../*.vhd &&
ghdl -m --workdir=work mono2stereo_adapter_tb &&
ghdl -r --workdir=work mono2stereo_adapter_tb --vcd=mono2stereo_adapter.vcd &&
gtkwave mono2stereo_adapter.vcd &
