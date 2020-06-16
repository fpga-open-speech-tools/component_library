# TCL File Generated by Component Editor 18.0
# Thu Oct 31 08:01:21 MDT 2019
# DO NOT MODIFY


# 
# FE_Speaker_Array_Encoder "FE_Speaker_Array_Encoder" v1.0
#  2019.10.31.08:01:21
# 
# 

# 
# request TCL package from ACDS 16.1
# 
package require -exact qsys 16.1


# 
# module FE_Speaker_Array_Encoder
# 
set_module_property DESCRIPTION ""
set_module_property NAME FE_Speaker_Array_Encoder
set_module_property VERSION 1.0
set_module_property INTERNAL false
set_module_property OPAQUE_ADDRESS_MAP true
set_module_property AUTHOR ""
set_module_property DISPLAY_NAME FE_Speaker_Array_Encoder
set_module_property INSTANTIATE_IN_SYSTEM_MODULE true
set_module_property EDITABLE true
set_module_property REPORT_TO_TALKBACK false
set_module_property ALLOW_GREYBOX_GENERATION false
set_module_property REPORT_HIERARCHY false
set_module_property GROUP "FPGA Open Speech Tools"

# 
# file sets
# 
add_fileset QUARTUS_SYNTH QUARTUS_SYNTH "" ""
set_fileset_property QUARTUS_SYNTH TOP_LEVEL Speaker_Array_Encoder
set_fileset_property QUARTUS_SYNTH ENABLE_RELATIVE_INCLUDE_PATHS false
set_fileset_property QUARTUS_SYNTH ENABLE_FILE_OVERWRITE_MODE false
add_fileset_file Speaker_Array_Encoder.vhd VHDL PATH Speaker_Array_Encoder.vhd TOP_LEVEL_FILE
add_fileset_file Parallel2Serial_32bits.qip OTHER PATH ../../serdes/Parallel2Serial_32bits.qip
add_fileset_file Array_DPR.qip OTHER PATH Array_DPR.qip
add_fileset_file Gen_Shift_Container.vhd VHDL PATH ../../I2S/Source_Files/Gen_Shift_Container.vhd


# 
# parameters
# 


# 
# display items
# 


# 
# connection point reset
# 
add_interface reset reset end
set_interface_property reset associatedClock sys_clk
set_interface_property reset synchronousEdges DEASSERT
set_interface_property reset ENABLED true
set_interface_property reset EXPORT_OF ""
set_interface_property reset PORT_NAME_MAP ""
set_interface_property reset CMSIS_SVD_VARIABLES ""
set_interface_property reset SVD_ADDRESS_GROUP ""

add_interface_port reset reset_n reset_n Input 1


# 
# connection point avalon_streaming_sink
# 
add_interface avalon_streaming_sink avalon_streaming end
set_interface_property avalon_streaming_sink associatedClock sys_clk
set_interface_property avalon_streaming_sink associatedReset reset
set_interface_property avalon_streaming_sink dataBitsPerSymbol 32
set_interface_property avalon_streaming_sink errorDescriptor ""
set_interface_property avalon_streaming_sink firstSymbolInHighOrderBits true
set_interface_property avalon_streaming_sink maxChannel 0
set_interface_property avalon_streaming_sink readyLatency 0
set_interface_property avalon_streaming_sink ENABLED true
set_interface_property avalon_streaming_sink EXPORT_OF ""
set_interface_property avalon_streaming_sink PORT_NAME_MAP ""
set_interface_property avalon_streaming_sink CMSIS_SVD_VARIABLES ""
set_interface_property avalon_streaming_sink SVD_ADDRESS_GROUP ""

add_interface_port avalon_streaming_sink data_input_channel channel Input 7
add_interface_port avalon_streaming_sink data_input_data data Input 32
add_interface_port avalon_streaming_sink data_input_error error Input 2
add_interface_port avalon_streaming_sink data_input_valid valid Input 1


# 
# connection point serial_output
# 
add_interface serial_output conduit end
set_interface_property serial_output associatedClock sys_clk
set_interface_property serial_output associatedReset ""
set_interface_property serial_output ENABLED true
set_interface_property serial_output EXPORT_OF ""
set_interface_property serial_output PORT_NAME_MAP ""
set_interface_property serial_output CMSIS_SVD_VARIABLES ""
set_interface_property serial_output SVD_ADDRESS_GROUP ""

add_interface_port serial_output serial_control serial_control Output 1
add_interface_port serial_output serial_data_out serial_data_out Output 1
add_interface_port serial_output clk_out clk_out Output 1


# 
# connection point led_output
# 
add_interface led_output conduit end
set_interface_property led_output associatedClock sys_clk
set_interface_property led_output associatedReset ""
set_interface_property led_output ENABLED true
set_interface_property led_output EXPORT_OF ""
set_interface_property led_output PORT_NAME_MAP ""
set_interface_property led_output CMSIS_SVD_VARIABLES ""
set_interface_property led_output SVD_ADDRESS_GROUP ""

add_interface_port led_output led_sd led_sd Output 1
add_interface_port led_output led_ws led_ws Output 1


# 
# connection point serial_clk
# 
add_interface serial_clk clock end
set_interface_property serial_clk clockRate 0
set_interface_property serial_clk ENABLED true
set_interface_property serial_clk EXPORT_OF ""
set_interface_property serial_clk PORT_NAME_MAP ""
set_interface_property serial_clk CMSIS_SVD_VARIABLES ""
set_interface_property serial_clk SVD_ADDRESS_GROUP ""

add_interface_port serial_clk serial_clk_in clk Input 1


# 
# connection point sys_clk
# 
add_interface sys_clk clock end
set_interface_property sys_clk clockRate 0
set_interface_property sys_clk ENABLED true
set_interface_property sys_clk EXPORT_OF ""
set_interface_property sys_clk PORT_NAME_MAP ""
set_interface_property sys_clk CMSIS_SVD_VARIABLES ""
set_interface_property sys_clk SVD_ADDRESS_GROUP ""

add_interface_port sys_clk sys_clk clk Input 1

