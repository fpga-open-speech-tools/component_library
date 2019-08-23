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
module  Serial2Parallel_32bits_lpm_shiftreg_180_gizevoi  (
	clock,
	shiftin,
	q);

	input	  clock;
	input	  shiftin;
	output	[31:0]  q;

	wire [31:0] sub_wire0;
	wire [31:0] q = sub_wire0[31:0];

	lpm_shiftreg  LPM_SHIFTREG_component (
				.clock (clock),
				.shiftin (shiftin),
				.q (sub_wire0)
				// synopsys translate_off
				,
				.aclr (),
				.aset (),
				.data (),
				.enable (),
				.load (),
				.sclr (),
				.shiftout (),
				.sset ()
				// synopsys translate_on
				);
	defparam
		LPM_SHIFTREG_component.lpm_direction  = "LEFT",
		LPM_SHIFTREG_component.lpm_type  = "LPM_SHIFTREG",
		LPM_SHIFTREG_component.lpm_width  = 32;


endmodule


