package require -exact qsys 14.0
create_system Parallel2Serial_32bits
add_instance Parallel2Serial_32bits lpm_shiftreg
set_instance_property Parallel2Serial_32bits AUTO_EXPORT true
set param_pairs [list]
lappend param_pairs {GUI_USE_CLKEN_INPUT} {false}
lappend param_pairs {DEVICE_FAMILY} {Arria 10}
lappend param_pairs {GUI_DIRECTION} {Left}
lappend param_pairs {GUI_USE_PARALLEL_INPUT} {true}
lappend param_pairs {GUI_USE_DATA_OUTPUT} {false}
lappend param_pairs {GUI_USE_SERIAL_INPUT} {false}
lappend param_pairs {GUI_USE_SERIAL_OUTPUT} {true}
lappend param_pairs {GUI_WIDTH} {32}
lappend param_pairs {GUI_USE_SCLR_INPUT} {false}
lappend param_pairs {GUI_USE_SSET_INPUT_1s} {false}
lappend param_pairs {GUI_USE_SSET_INPUT} {false}
lappend param_pairs {GUI_USE_ACLR_INPUT} {false}
lappend param_pairs {GUI_USE_ASET_INPUT_1s} {false}
lappend param_pairs {GUI_USE_ASET_INPUT} {false}
foreach {param_name param_value} $param_pairs {
    set sys_info_type [get_instance_parameter_property Parallel2Serial_32bits "$param_name" SYSTEM_INFO_TYPE]
    if {$sys_info_type == "DEVICE_FAMILY"} {
        set_project_property DEVICE_FAMILY "$param_value"
        break;
    }
}
foreach {param_name param_value} $param_pairs {
    set_instance_parameter_value Parallel2Serial_32bits "$param_name" "$param_value"
}
save_system {C:/Users/tyler/Documents/NIHIIRepository/NIH2/Quartus_180/Library/Platform_Designer/AD1939/Source_Files/Parallel2Serial_32bits.qsys}
