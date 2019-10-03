// (C) 2001-2018 Intel Corporation. All rights reserved.
// Your use of Intel Corporation's design tools, logic functions and other 
// software and tools, and its AMPP partner logic functions, and any output 
// files from any of the foregoing (including device programming or simulation 
// files), and any associated documentation or information are expressly subject 
// to the terms and conditions of the Intel Program License Subscription 
// Agreement, Intel FPGA IP License Agreement, or other applicable 
// license agreement, including, without limitation, that your use is for the 
// sole purpose of programming logic devices manufactured by Intel and sold by 
// Intel or its authorized distributors.  Please refer to the applicable 
// agreement for further details.



// synopsys translate_off
`timescale 1 ps / 1 ps
// synopsys translate_on
module  Parallel2Serial_32bits_lpm_shiftreg_180_wxh4spi  (
	clock,
	data,
	load,
	shiftout);

	input	  clock;
	input	[31:0]  data;
	input	  load;
	output	shiftout;

	wire sub_wire0;
	wire shiftout = sub_wire0;

	lpm_shiftreg  LPM_SHIFTREG_component (
				.clock (clock),
				.data (data),
				.load (load),
				.shiftout (sub_wire0)
				// synopsys translate_off
				,
				.aclr (),
				.aset (),
				.enable (),
				.q (),
				.sclr (),
				.shiftin (),
				.sset ()
				// synopsys translate_on
				);
	defparam
		LPM_SHIFTREG_component.lpm_direction  = "LEFT",
		LPM_SHIFTREG_component.lpm_type  = "LPM_SHIFTREG",
		LPM_SHIFTREG_component.lpm_width  = 32;


endmodule


