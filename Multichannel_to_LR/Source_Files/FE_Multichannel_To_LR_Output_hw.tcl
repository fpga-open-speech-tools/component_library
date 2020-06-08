# TCL File Generated by Component Editor 18.0
# Mon Nov 05 11:25:58 MST 2018
# DO NOT MODIFY


# 
# FE_Multichannel_To_LR_Output "FE_Multichannel_To_LR_Output" v1.0
#  2018.11.05.11:25:58
# 
# 

# 
# request TCL package from ACDS 16.1
# 
package require -exact qsys 16.1


# 
# module FE_Multichannel_To_LR_Output
# 
set_module_property DESCRIPTION ""
set_module_property NAME FE_Multichannel_To_LR_Output
set_module_property VERSION 1.0
set_module_property INTERNAL false
set_module_property OPAQUE_ADDRESS_MAP true
set_module_property AUTHOR ""
set_module_property DISPLAY_NAME FE_Multichannel_To_LR_Output
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
set_fileset_property QUARTUS_SYNTH TOP_LEVEL FE_Multichannel_To_LR_Output
set_fileset_property QUARTUS_SYNTH ENABLE_RELATIVE_INCLUDE_PATHS false
set_fileset_property QUARTUS_SYNTH ENABLE_FILE_OVERWRITE_MODE false
add_fileset_file FE_Multichannel_To_LR_Output.vhd VHDL PATH FE_Multichannel_To_LR_Output.vhd TOP_LEVEL_FILE

#
# module assignments
# 
set_module_assignment embeddedsw.dts.compatible dev,fe-multichannel-to-LR
set_module_assignment embeddedsw.dts.group multichannel-to-LR
set_module_assignment embeddedsw.dts.vendor fe

# 
# parameters
# 


# 
# display items
# 


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


# 
# connection point sys_reset
# 
add_interface sys_reset reset end
set_interface_property sys_reset associatedClock sys_clk
set_interface_property sys_reset synchronousEdges DEASSERT
set_interface_property sys_reset ENABLED true
set_interface_property sys_reset EXPORT_OF ""
set_interface_property sys_reset PORT_NAME_MAP ""
set_interface_property sys_reset CMSIS_SVD_VARIABLES ""
set_interface_property sys_reset SVD_ADDRESS_GROUP ""

add_interface_port sys_reset sys_reset reset Input 1


# 
# connection point multiplex_data_in
# 
add_interface multiplex_data_in avalon_streaming end
set_interface_property multiplex_data_in associatedClock sys_clk
set_interface_property multiplex_data_in associatedReset sys_reset
set_interface_property multiplex_data_in dataBitsPerSymbol 8
set_interface_property multiplex_data_in errorDescriptor ""
set_interface_property multiplex_data_in firstSymbolInHighOrderBits true
set_interface_property multiplex_data_in maxChannel 0
set_interface_property multiplex_data_in readyLatency 0
set_interface_property multiplex_data_in ENABLED true
set_interface_property multiplex_data_in EXPORT_OF ""
set_interface_property multiplex_data_in PORT_NAME_MAP ""
set_interface_property multiplex_data_in CMSIS_SVD_VARIABLES ""
set_interface_property multiplex_data_in SVD_ADDRESS_GROUP ""

add_interface_port multiplex_data_in multiplex_data_in_data data Input 32
add_interface_port multiplex_data_in multiplex_data_in_valid valid Input 1
add_interface_port multiplex_data_in multiplex_data_in_error error Input 2
add_interface_port multiplex_data_in multiplex_data_in_channel channel Input 2


# 
# connection point data_out_left
# 
add_interface data_out_left avalon_streaming start
set_interface_property data_out_left associatedClock sys_clk
set_interface_property data_out_left associatedReset sys_reset
set_interface_property data_out_left dataBitsPerSymbol 8
set_interface_property data_out_left errorDescriptor ""
set_interface_property data_out_left firstSymbolInHighOrderBits true
set_interface_property data_out_left maxChannel 0
set_interface_property data_out_left readyLatency 0
set_interface_property data_out_left ENABLED true
set_interface_property data_out_left EXPORT_OF ""
set_interface_property data_out_left PORT_NAME_MAP ""
set_interface_property data_out_left CMSIS_SVD_VARIABLES ""
set_interface_property data_out_left SVD_ADDRESS_GROUP ""

add_interface_port data_out_left data_out_left_data data Output 32
add_interface_port data_out_left data_out_left_error error Output 2
add_interface_port data_out_left data_out_left_valid valid Output 1


# 
# connection point data_out_right
# 
add_interface data_out_right avalon_streaming start
set_interface_property data_out_right associatedClock sys_clk
set_interface_property data_out_right associatedReset sys_reset
set_interface_property data_out_right dataBitsPerSymbol 8
set_interface_property data_out_right errorDescriptor ""
set_interface_property data_out_right firstSymbolInHighOrderBits true
set_interface_property data_out_right maxChannel 0
set_interface_property data_out_right readyLatency 0
set_interface_property data_out_right ENABLED true
set_interface_property data_out_right EXPORT_OF ""
set_interface_property data_out_right PORT_NAME_MAP ""
set_interface_property data_out_right CMSIS_SVD_VARIABLES ""
set_interface_property data_out_right SVD_ADDRESS_GROUP ""

add_interface_port data_out_right data_out_right_data data Output 32
add_interface_port data_out_right data_out_right_error error Output 2
add_interface_port data_out_right data_out_right_valid valid Output 1

