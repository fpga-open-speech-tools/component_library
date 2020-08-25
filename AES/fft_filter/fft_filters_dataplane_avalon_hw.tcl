# # # # # # # # # # # # # # # # # #
# Built in create_header_file_stuff
# # # # # # # # # # # # # # # # # #

package require -exact qsys 16.1
# End create_header_file_stuff


# # # # # # # # # # # # # # # # #
# Created in create_module
# # # # # # # # # # # # # # # # #

set_module_property DESCRIPTION ""
set_module_property NAME "fft_filters"
set_module_property VERSION 1.0
set_module_property OPAQUE_ADDRESS_MAP true
set_module_property AUTHOR ""
set_module_property GROUP "FPGA Open Speech Tools/Autogen"
set_module_property DISPLAY_NAME "fft_filters"
set_module_property INSTANTIATE_IN_SYSTEM_MODULE true
set_module_property EDITABLE true
set_module_property REPORT_TO_TALKBACK false
set_module_property ALLOW_GREYBOX_GENERATION false
set_module_property VERSION 1.0
set_module_property REPORT_HIERARCHY false
# end of create_module


# # # # # # # # # # # # # # # # # #
# created in create_file_sets
# # # # # # # # # # # # # # # # # #

add_fileset QUARTUS_SYNTH QUARTUS_SYNTH "" ""
add_fileset_file fft_filters_addr_A_offset.vhd VHDL PATH fft_filters_addr_A_offset.vhd
add_fileset_file fft_filters_addr_A_offset_block.vhd VHDL PATH fft_filters_addr_A_offset_block.vhd
add_fileset_file fft_filters_addr_B_gen.vhd VHDL PATH fft_filters_addr_B_gen.vhd
add_fileset_file fft_filters_addr_B_gen_block.vhd VHDL PATH fft_filters_addr_B_gen_block.vhd
add_fileset_file fft_filters_addr_B_gen_enable.vhd VHDL PATH fft_filters_addr_B_gen_enable.vhd
add_fileset_file fft_filters_addr_B_gen_enable_block.vhd VHDL PATH fft_filters_addr_B_gen_enable_block.vhd
add_fileset_file fft_filters_Analysis.vhd VHDL PATH fft_filters_Analysis.vhd
add_fileset_file fft_filters_Analysis_block.vhd VHDL PATH fft_filters_Analysis_block.vhd
add_fileset_file fft_filters_Apply_Complex_Gains.vhd VHDL PATH fft_filters_Apply_Complex_Gains.vhd
add_fileset_file fft_filters_Apply_Complex_Gains_block.vhd VHDL PATH fft_filters_Apply_Complex_Gains_block.vhd
add_fileset_file fft_filters_Complex4Multiply.vhd VHDL PATH fft_filters_Complex4Multiply.vhd
add_fileset_file fft_filters_Complex4Multiply_block.vhd VHDL PATH fft_filters_Complex4Multiply_block.vhd
add_fileset_file fft_filters_Complex4Multiply_block1.vhd VHDL PATH fft_filters_Complex4Multiply_block1.vhd
add_fileset_file fft_filters_Complex4Multiply_block2.vhd VHDL PATH fft_filters_Complex4Multiply_block2.vhd
add_fileset_file fft_filters_Complex_Gains_1_ROM.vhd VHDL PATH fft_filters_Complex_Gains_1_ROM.vhd
add_fileset_file fft_filters_Complex_Gains_1_ROM_block.vhd VHDL PATH fft_filters_Complex_Gains_1_ROM_block.vhd
add_fileset_file fft_filters_Complex_Gains_2_ROM.vhd VHDL PATH fft_filters_Complex_Gains_2_ROM.vhd
add_fileset_file fft_filters_Complex_Gains_2_ROM_block.vhd VHDL PATH fft_filters_Complex_Gains_2_ROM_block.vhd
add_fileset_file fft_filters_Complex_Gains_3_ROM.vhd VHDL PATH fft_filters_Complex_Gains_3_ROM.vhd
add_fileset_file fft_filters_Complex_Gains_3_ROM_block.vhd VHDL PATH fft_filters_Complex_Gains_3_ROM_block.vhd
add_fileset_file fft_filters_Complex_Gains_4_ROM.vhd VHDL PATH fft_filters_Complex_Gains_4_ROM.vhd
add_fileset_file fft_filters_Complex_Gains_4_ROM_block.vhd VHDL PATH fft_filters_Complex_Gains_4_ROM_block.vhd
add_fileset_file fft_filters_dataplane.vhd VHDL PATH fft_filters_dataplane.vhd
add_fileset_file fft_filters_dataplane_pkg.vhd VHDL PATH fft_filters_dataplane_pkg.vhd
add_fileset_file fft_filters_dataplane_tc.vhd VHDL PATH fft_filters_dataplane_tc.vhd
add_fileset_file fft_filters_DualRateDualPortRAM_generic.vhd VHDL PATH fft_filters_DualRateDualPortRAM_generic.vhd
add_fileset_file fft_filters_Fast_Transition.vhd VHDL PATH fft_filters_Fast_Transition.vhd
add_fileset_file fft_filters_Fast_Transition_block.vhd VHDL PATH fft_filters_Fast_Transition_block.vhd
add_fileset_file fft_filters_FFT.vhd VHDL PATH fft_filters_FFT.vhd
add_fileset_file fft_filters_FFT_Analysis_Synthesis_Left.vhd VHDL PATH fft_filters_FFT_Analysis_Synthesis_Left.vhd
add_fileset_file fft_filters_FFT_Analysis_Synthesis_Right.vhd VHDL PATH fft_filters_FFT_Analysis_Synthesis_Right.vhd
add_fileset_file fft_filters_FFT_block.vhd VHDL PATH fft_filters_FFT_block.vhd
add_fileset_file fft_filters_FFT_Filter_Coefficients.vhd VHDL PATH fft_filters_FFT_Filter_Coefficients.vhd
add_fileset_file fft_filters_FFT_Filter_Coefficients_block.vhd VHDL PATH fft_filters_FFT_Filter_Coefficients_block.vhd
add_fileset_file fft_filters_FFT_Frame_Buffering.vhd VHDL PATH fft_filters_FFT_Frame_Buffering.vhd
add_fileset_file fft_filters_FFT_Frame_Buffering_block.vhd VHDL PATH fft_filters_FFT_Frame_Buffering_block.vhd
add_fileset_file fft_filters_FFT_Frame_Pulse_Gen.vhd VHDL PATH fft_filters_FFT_Frame_Pulse_Gen.vhd
add_fileset_file fft_filters_FFT_Frame_Pulse_Gen_block.vhd VHDL PATH fft_filters_FFT_Frame_Pulse_Gen_block.vhd
add_fileset_file fft_filters_FFT_pulse_gen.vhd VHDL PATH fft_filters_FFT_pulse_gen.vhd
add_fileset_file fft_filters_FFT_pulse_gen_block.vhd VHDL PATH fft_filters_FFT_pulse_gen_block.vhd
add_fileset_file fft_filters_FFT_ROM_Indexing.vhd VHDL PATH fft_filters_FFT_ROM_Indexing.vhd
add_fileset_file fft_filters_FFT_ROM_Indexing_block.vhd VHDL PATH fft_filters_FFT_ROM_Indexing_block.vhd
add_fileset_file fft_filters_FIFO_1.vhd VHDL PATH fft_filters_FIFO_1.vhd
add_fileset_file fft_filters_FIFO_1_block.vhd VHDL PATH fft_filters_FIFO_1_block.vhd
add_fileset_file fft_filters_FIFO_2.vhd VHDL PATH fft_filters_FIFO_2.vhd
add_fileset_file fft_filters_FIFO_2_block.vhd VHDL PATH fft_filters_FIFO_2_block.vhd
add_fileset_file fft_filters_FIFO_3.vhd VHDL PATH fft_filters_FIFO_3.vhd
add_fileset_file fft_filters_FIFO_3_block.vhd VHDL PATH fft_filters_FIFO_3_block.vhd
add_fileset_file fft_filters_FIFO_4.vhd VHDL PATH fft_filters_FIFO_4.vhd
add_fileset_file fft_filters_FIFO_4_block.vhd VHDL PATH fft_filters_FIFO_4_block.vhd
add_fileset_file fft_filters_fifo_state_machine.vhd VHDL PATH fft_filters_fifo_state_machine.vhd
add_fileset_file fft_filters_fifo_state_machine1.vhd VHDL PATH fft_filters_fifo_state_machine1.vhd
add_fileset_file fft_filters_fifo_state_machine1_block.vhd VHDL PATH fft_filters_fifo_state_machine1_block.vhd
add_fileset_file fft_filters_fifo_state_machine2.vhd VHDL PATH fft_filters_fifo_state_machine2.vhd
add_fileset_file fft_filters_fifo_state_machine2_block.vhd VHDL PATH fft_filters_fifo_state_machine2_block.vhd
add_fileset_file fft_filters_fifo_state_machine3.vhd VHDL PATH fft_filters_fifo_state_machine3.vhd
add_fileset_file fft_filters_fifo_state_machine3_block.vhd VHDL PATH fft_filters_fifo_state_machine3_block.vhd
add_fileset_file fft_filters_fifo_state_machine_block.vhd VHDL PATH fft_filters_fifo_state_machine_block.vhd
add_fileset_file fft_filters_FIFO_Write_Select.vhd VHDL PATH fft_filters_FIFO_Write_Select.vhd
add_fileset_file fft_filters_FIFO_Write_Select_block.vhd VHDL PATH fft_filters_FIFO_Write_Select_block.vhd
add_fileset_file fft_filters_Filter_ROM_Choice.vhd VHDL PATH fft_filters_Filter_ROM_Choice.vhd
add_fileset_file fft_filters_Filter_ROM_Choice_block.vhd VHDL PATH fft_filters_Filter_ROM_Choice_block.vhd
add_fileset_file fft_filters_Frequency_Domain_Processing.vhd VHDL PATH fft_filters_Frequency_Domain_Processing.vhd
add_fileset_file fft_filters_Frequency_Domain_Processing_block.vhd VHDL PATH fft_filters_Frequency_Domain_Processing_block.vhd
add_fileset_file fft_filters_Gains_ROM_imag.vhd VHDL PATH fft_filters_Gains_ROM_imag.vhd
add_fileset_file fft_filters_Gains_ROM_imag_block.vhd VHDL PATH fft_filters_Gains_ROM_imag_block.vhd
add_fileset_file fft_filters_Gains_ROM_imag_block1.vhd VHDL PATH fft_filters_Gains_ROM_imag_block1.vhd
add_fileset_file fft_filters_Gains_ROM_imag_block2.vhd VHDL PATH fft_filters_Gains_ROM_imag_block2.vhd
add_fileset_file fft_filters_Gains_ROM_imag_block3.vhd VHDL PATH fft_filters_Gains_ROM_imag_block3.vhd
add_fileset_file fft_filters_Gains_ROM_imag_block4.vhd VHDL PATH fft_filters_Gains_ROM_imag_block4.vhd
add_fileset_file fft_filters_Gains_ROM_imag_block5.vhd VHDL PATH fft_filters_Gains_ROM_imag_block5.vhd
add_fileset_file fft_filters_Gains_ROM_imag_block6.vhd VHDL PATH fft_filters_Gains_ROM_imag_block6.vhd
add_fileset_file fft_filters_Gains_ROM_real.vhd VHDL PATH fft_filters_Gains_ROM_real.vhd
add_fileset_file fft_filters_Gains_ROM_real_block.vhd VHDL PATH fft_filters_Gains_ROM_real_block.vhd
add_fileset_file fft_filters_Gains_ROM_real_block1.vhd VHDL PATH fft_filters_Gains_ROM_real_block1.vhd
add_fileset_file fft_filters_Gains_ROM_real_block2.vhd VHDL PATH fft_filters_Gains_ROM_real_block2.vhd
add_fileset_file fft_filters_Gains_ROM_real_block3.vhd VHDL PATH fft_filters_Gains_ROM_real_block3.vhd
add_fileset_file fft_filters_Gains_ROM_real_block4.vhd VHDL PATH fft_filters_Gains_ROM_real_block4.vhd
add_fileset_file fft_filters_Gains_ROM_real_block5.vhd VHDL PATH fft_filters_Gains_ROM_real_block5.vhd
add_fileset_file fft_filters_Gains_ROM_real_block6.vhd VHDL PATH fft_filters_Gains_ROM_real_block6.vhd
add_fileset_file fft_filters_Hanning_ROM.vhd VHDL PATH fft_filters_Hanning_ROM.vhd
add_fileset_file fft_filters_Hanning_ROM_block.vhd VHDL PATH fft_filters_Hanning_ROM_block.vhd
add_fileset_file fft_filters_Hanning_ROM_block1.vhd VHDL PATH fft_filters_Hanning_ROM_block1.vhd
add_fileset_file fft_filters_Hanning_ROM_block2.vhd VHDL PATH fft_filters_Hanning_ROM_block2.vhd
add_fileset_file fft_filters_iFFT.vhd VHDL PATH fft_filters_iFFT.vhd
add_fileset_file fft_filters_iFFT_block.vhd VHDL PATH fft_filters_iFFT_block.vhd
add_fileset_file fft_filters_MINRESRX2FFT_BTFSEL.vhd VHDL PATH fft_filters_MINRESRX2FFT_BTFSEL.vhd
add_fileset_file fft_filters_MINRESRX2FFT_BTFSEL_block.vhd VHDL PATH fft_filters_MINRESRX2FFT_BTFSEL_block.vhd
add_fileset_file fft_filters_MINRESRX2FFT_BTFSEL_block1.vhd VHDL PATH fft_filters_MINRESRX2FFT_BTFSEL_block1.vhd
add_fileset_file fft_filters_MINRESRX2FFT_BTFSEL_block2.vhd VHDL PATH fft_filters_MINRESRX2FFT_BTFSEL_block2.vhd
add_fileset_file fft_filters_MINRESRX2FFT_CTRL.vhd VHDL PATH fft_filters_MINRESRX2FFT_CTRL.vhd
add_fileset_file fft_filters_MINRESRX2FFT_CTRL_block.vhd VHDL PATH fft_filters_MINRESRX2FFT_CTRL_block.vhd
add_fileset_file fft_filters_MINRESRX2FFT_CTRL_block1.vhd VHDL PATH fft_filters_MINRESRX2FFT_CTRL_block1.vhd
add_fileset_file fft_filters_MINRESRX2FFT_CTRL_block2.vhd VHDL PATH fft_filters_MINRESRX2FFT_CTRL_block2.vhd
add_fileset_file fft_filters_MINRESRX2FFT_MEMORY.vhd VHDL PATH fft_filters_MINRESRX2FFT_MEMORY.vhd
add_fileset_file fft_filters_MINRESRX2FFT_MEMORY_block.vhd VHDL PATH fft_filters_MINRESRX2FFT_MEMORY_block.vhd
add_fileset_file fft_filters_MINRESRX2FFT_MEMORY_block1.vhd VHDL PATH fft_filters_MINRESRX2FFT_MEMORY_block1.vhd
add_fileset_file fft_filters_MINRESRX2FFT_MEMORY_block2.vhd VHDL PATH fft_filters_MINRESRX2FFT_MEMORY_block2.vhd
add_fileset_file fft_filters_MINRESRX2FFT_MEMSEL.vhd VHDL PATH fft_filters_MINRESRX2FFT_MEMSEL.vhd
add_fileset_file fft_filters_MINRESRX2FFT_MEMSEL_block.vhd VHDL PATH fft_filters_MINRESRX2FFT_MEMSEL_block.vhd
add_fileset_file fft_filters_MINRESRX2FFT_MEMSEL_block1.vhd VHDL PATH fft_filters_MINRESRX2FFT_MEMSEL_block1.vhd
add_fileset_file fft_filters_MINRESRX2FFT_MEMSEL_block2.vhd VHDL PATH fft_filters_MINRESRX2FFT_MEMSEL_block2.vhd
add_fileset_file fft_filters_MINRESRX2FFT_OUTMux.vhd VHDL PATH fft_filters_MINRESRX2FFT_OUTMux.vhd
add_fileset_file fft_filters_MINRESRX2FFT_OUTMux_block.vhd VHDL PATH fft_filters_MINRESRX2FFT_OUTMux_block.vhd
add_fileset_file fft_filters_MINRESRX2FFT_OUTMux_block1.vhd VHDL PATH fft_filters_MINRESRX2FFT_OUTMux_block1.vhd
add_fileset_file fft_filters_MINRESRX2FFT_OUTMux_block2.vhd VHDL PATH fft_filters_MINRESRX2FFT_OUTMux_block2.vhd
add_fileset_file fft_filters_MINRESRX2_BUTTERFLY.vhd VHDL PATH fft_filters_MINRESRX2_BUTTERFLY.vhd
add_fileset_file fft_filters_MINRESRX2_BUTTERFLY_block.vhd VHDL PATH fft_filters_MINRESRX2_BUTTERFLY_block.vhd
add_fileset_file fft_filters_MINRESRX2_BUTTERFLY_block1.vhd VHDL PATH fft_filters_MINRESRX2_BUTTERFLY_block1.vhd
add_fileset_file fft_filters_MINRESRX2_BUTTERFLY_block2.vhd VHDL PATH fft_filters_MINRESRX2_BUTTERFLY_block2.vhd
add_fileset_file fft_filters_Overlap_and_Add.vhd VHDL PATH fft_filters_Overlap_and_Add.vhd
add_fileset_file fft_filters_Overlap_and_Add_block.vhd VHDL PATH fft_filters_Overlap_and_Add_block.vhd
add_fileset_file fft_filters_SimpleDualPortRAM_generic.vhd VHDL PATH fft_filters_SimpleDualPortRAM_generic.vhd
add_fileset_file fft_filters_Synthesis.vhd VHDL PATH fft_filters_Synthesis.vhd
add_fileset_file fft_filters_Synthesis_block.vhd VHDL PATH fft_filters_Synthesis_block.vhd
add_fileset_file fft_filters_TWDLROM.vhd VHDL PATH fft_filters_TWDLROM.vhd
add_fileset_file fft_filters_TWDLROM_block.vhd VHDL PATH fft_filters_TWDLROM_block.vhd
add_fileset_file fft_filters_TWDLROM_block1.vhd VHDL PATH fft_filters_TWDLROM_block1.vhd
add_fileset_file fft_filters_TWDLROM_block2.vhd VHDL PATH fft_filters_TWDLROM_block2.vhd
add_fileset_file fft_filters_Wait_for_data_to_start.vhd VHDL PATH fft_filters_Wait_for_data_to_start.vhd
add_fileset_file fft_filters_Wait_for_data_to_start_block.vhd VHDL PATH fft_filters_Wait_for_data_to_start_block.vhd
add_fileset_file fixed_resize_pkg.vhd VHDL PATH fixed_resize_pkg.vhd
set_fileset_property QUARTUS_SYNTH TOP_LEVEL fft_filters_dataplane_avalon
set_fileset_property QUARTUS_SYNTH ENABLE_RELATIVE_INCLUDE_PATHS false
set_fileset_property QUARTUS_SYNTH ENABLE_FILE_OVERWRITE_MODE false
add_fileset_file fft_filters_dataplane_avalon.vhd VHDL PATH fft_filters_dataplane_avalon.vhd TOP_LEVEL_FILE
# end create_file_sets


# # # # # # # # # # # # # # # # # #
# Created in create_module_assignments
# # # # # # # # # # # # # # # # # # # #

set_module_assignment embeddedsw.dts.compatible dev,al-fft_filters
set_module_assignment embeddedsw.dts.group autogen 
set_module_assignment embeddedsw.dts.vendor al
# End create_module_assignments


# # # # # # # # # # # # # # # # # # # # # #
# Created by create_connection_point_clock
# # # # # # # # # # # # # # # # # # # # # #

add_interface clock clock end
set_interface_property clock clockRate 98304000
set_interface_property clock ENABLED true
set_interface_property clock EXPORT_OF ""
set_interface_property clock PORT_NAME_MAP ""
set_interface_property clock CMSIS_SVD_VARIABLES ""
set_interface_property clock SVD_ADDRESS_GROUP ""
add_interface_port clock clk clk Input 1
# End create_connection_point_clock


# # # # # # # # # # # # # # # # # # # # # #
# Created by create_connection_point_reset
# # # # # # # # # # # # # # # # # # # # # #

add_interface reset reset end
set_interface_property reset associatedClock clock
set_interface_property reset synchronousEdges DEASSERT
set_interface_property reset ENABLED true
set_interface_property reset EXPORT_OF true
set_interface_property reset PORT_NAME_MAP ""
set_interface_property reset CMSIS_SVD_VARIABLES ""
set_interface_property reset SVD_ADDRESS_GROUP ""
add_interface_port reset reset reset Input 1
# End create_connection_point_reset


# # # # # # # # # # # # # # # # # # # # #
# Created by create_mm_connection_point
# # # # # # # # # # # # # # # # # # # # #

add_interface avalon_slave avalon end
set_interface_property avalon_slave addressUnits WORDS
set_interface_property avalon_slave associatedClock clock
set_interface_property avalon_slave associatedReset reset
set_interface_property avalon_slave bitsPerSymbol 8
set_interface_property avalon_slave burstOnBurstBoundariesOnly false
set_interface_property avalon_slave burstcountUnits WORDS
set_interface_property avalon_slave explicitAddressSpan 0
set_interface_property avalon_slave holdTime 0
set_interface_property avalon_slave linewrapBursts false
set_interface_property avalon_slave maximumPendingReadTransactions 0
set_interface_property avalon_slave maximumPendingWriteTransactions 0
set_interface_property avalon_slave readLatency 0
set_interface_property avalon_slave readWaitTime 1
set_interface_property avalon_slave setupTime 1
set_interface_property avalon_slave timingUnits Cycles
set_interface_property avalon_slave writeWaitTime 0
set_interface_property avalon_slave ENABLED true
set_interface_property avalon_slave EXPORT_OF ""
set_interface_property avalon_slave PORT_NAME_MAP ""
set_interface_property avalon_slave CMSIS_SVD_VARIABLES ""
set_interface_property avalon_slave SVD_ADDRESS_GROUP ""

add_interface_port avalon_slave avalon_slave_address address Input 1
add_interface_port avalon_slave avalon_slave_read read Input 1
add_interface_port avalon_slave avalon_slave_readdata readdata Output 32
add_interface_port avalon_slave avalon_slave_write write Input 1
add_interface_port avalon_slave avalon_slave_writedata writedata Input 32
set_interface_assignment avalon_slave embeddedsw.configuration.isFlash 0
set_interface_assignment avalon_slave embeddedsw.configuration.isMemoryDevice 0
set_interface_assignment avalon_slave embeddedsw.configuration.isNonVolatileStorage 0
set_interface_assignment avalon_slave embeddedsw.configuration.isPrintableDevice 0
# End create_mm_connection_point


# # # # # # # # # # # # # # # # # # # # # #
# Created by create_sink_connection_point
# # # # # # # # # # # # # # # # # # # # # #

add_interface avalon_streaming_sink avalon_streaming end
set_interface_property avalon_streaming_sink associatedClock clock
set_interface_property avalon_streaming_sink associatedReset reset
set_interface_property avalon_streaming_sink dataBitsPerSymbol 32
set_interface_property avalon_streaming_sink errorDescriptor ""
set_interface_property avalon_streaming_sink firstSymbolInHighOrderBits true
set_interface_property avalon_streaming_sink maxChannel 3
set_interface_property avalon_streaming_sink readyLatency 0
set_interface_property avalon_streaming_sink ENABLED true
set_interface_property avalon_streaming_sink EXPORT_OF ""
set_interface_property avalon_streaming_sink PORT_NAME_MAP ""
set_interface_property avalon_streaming_sink CMSIS_SVD_VARIABLES ""
set_interface_property avalon_streaming_sink SVD_ADDRESS_GROUP ""
add_interface_port avalon_streaming_sink avalon_sink_valid valid Input 1
add_interface_port avalon_streaming_sink avalon_sink_data data Input 32
add_interface_port avalon_streaming_sink avalon_sink_channel channel Input 2
add_interface_port avalon_streaming_sink avalon_sink_error error Input 2
# End create_sink_connection_point


# # # # # # # # # # # # # # # # # # # # # # #
# Created in create_source_connection_point
# # # # # # # # # # # # # # # # # # # # # # #

add_interface avalon_streaming_source avalon_streaming start
set_interface_property avalon_streaming_source associatedClock clock
set_interface_property avalon_streaming_source associatedReset reset
set_interface_property avalon_streaming_source dataBitsPerSymbol 32
set_interface_property avalon_streaming_source errorDescriptor ""
set_interface_property avalon_streaming_source firstSymbolInHighOrderBits true
set_interface_property avalon_streaming_source maxChannel 3
set_interface_property avalon_streaming_source readyLatency 0
set_interface_property avalon_streaming_source ENABLED true
set_interface_property avalon_streaming_source EXPORT_OF ""
set_interface_property avalon_streaming_source PORT_NAME_MAP ""
set_interface_property avalon_streaming_source CMSIS_SVD_VARIABLES ""
set_interface_property avalon_streaming_source SVD_ADDRESS_GROUP ""
add_interface_port avalon_streaming_source avalon_source_valid valid Output 1
add_interface_port avalon_streaming_source avalon_source_data data Output 32
add_interface_port avalon_streaming_source avalon_source_channel channel Output 2
add_interface_port avalon_streaming_source avalon_source_error error Output 2
# End create_sink_connection_point


