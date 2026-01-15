#===============================================
# Top Text
#   
#===============================================


set prj_name [lindex $argv 0]
set prj_defs [lindex $argv 1]
puts "Project name: $prj_name"
puts "Defines: $prj_defs"
cd prj/$prj_name
set path_out ../../build

tclapp::install -quiet ultrafast
set part_name xc7z010clg400-1


################################################### BUILD AND PACKAGE CORES ######################################
cd cores
set core_names [glob -type d *]
cd ..

foreach core_name $core_names {
    set elements [split $core_name _]
    set project_name [join [lrange $elements 0 end-2] _]
    set version [string trimleft [join [lrange $elements end-1 end] .] v]

    file delete -force tmp/cores/$core_name tmp/cores/$project_name.cache tmp/cores/$project_name.hw tmp/cores/$project_name.xpr tmp/cores/$project_name.sim
    create_project -force -part $part_name $project_name tmp/cores

    add_files -norecurse [glob cores/$core_name/*.v]

    ipx::package_project -import_files -root_dir tmp/cores/$core_name

    set core [ipx::current_core]

    set_property VERSION $version $core
    set_property NAME $project_name $core
    set_property LIBRARY {user} $core
    set_property SUPPORTED_FAMILIES {zynq Production} $core

    proc core_parameter {name display_name description} {
      set core [ipx::current_core]

      set parameter [ipx::get_user_parameters $name -of_objects $core]
      set_property DISPLAY_NAME $display_name $parameter
      set_property DESCRIPTION $description $parameter

      set parameter [ipgui::get_guiparamspec -name $name -component $core]
      set_property DISPLAY_NAME $display_name $parameter
      set_property TOOLTIP $description $parameter
    }

    source cores/$core_name/core_config.tcl

    rename core_parameter {}

    ipx::create_xgui_files $core
    ipx::update_checksums $core
    ipx::save_core $core

    close_project
}



#################### CREATE PROJECT #############################################################################
set bd_path tmp/$prj_name/$prj_name.srcs/sources_1/bd/system

file delete -force tmp/$prj_name
create_project $prj_name tmp/$prj_name -part $part_name
create_bd_design system



#################### ADD SOURCE AND CONSTRAINT FILES ###########################################################
add_files -norecurse rtl

#set_msg_config -id {HDL 9-1061} -limit 1000
#set_msg_config -id {HDL 9-1332} -limit 1000
#synth_design -top pdh_core -rtl -name rtl_1



add_files -norecurse -fileset constrs_1 cfg/ports.xdc
add_files -norecurse -fileset constrs_1 cfg/clocks.xdc




################### ADD PORTS ##################################################################################

### ADC
create_bd_port -dir O adc_enc_p_o
create_bd_port -dir O adc_enc_n_o





### LED
#create_bd_port -dir O -from 7 -to 0 led_o





################### CONNECT CORES #########################################################################
set_property IP_REPO_PATHS tmp/cores [current_project]
update_ip_catalog

# Zynq processing system with RedPitaya specific preset
startgroup
create_bd_cell -type ip -vlnv xilinx.com:ip:processing_system7 processing_system7_0
set_property -dict [list CONFIG.PCW_USE_S_AXI_HP0 {1}] [get_bd_cells processing_system7_0]
set_property -dict [list CONFIG.PCW_IMPORT_BOARD_PRESET {cfg/red_pitaya.xml}] [get_bd_cells processing_system7_0]
endgroup


# AXI GPIO IP core
startgroup
create_bd_cell -type ip -vlnv xilinx.com:ip:axi_gpio axi_gpio_0
set_property -dict [list CONFIG.C_IS_DUAL {1} CONFIG.C_ALL_INPUTS_2 {1}] [get_bd_cells axi_gpio_0]
endgroup


#################### BUILD TOP-LEVEL WRAPPER AND SPECIFY LINK TO PS #####################################################


#Expose asic-level signals to the FPGA netlist
apply_bd_automation -rule xilinx.com:bd_rule:processing_system7 -config {make_external "FIXED_IO, DDR" Master "Disable" Slave "Disable" }  [get_bd_cells processing_system7_0]

#Clock master and slave AXI FSM's via PS FCLK
connect_bd_net [get_bd_pins processing_system7_0/M_AXI_GP0_ACLK] [get_bd_pins processing_system7_0/FCLK_CLK0]
connect_bd_net [get_bd_pins processing_system7_0/S_AXI_HP0_ACLK] [get_bd_pins processing_system7_0/FCLK_CLK0]


# Using the AXI GPIO logic to route signals between RTL and the PS
apply_bd_automation -rule xilinx.com:bd_rule:axi4 -config {Master "/processing_system7_0/M_AXI_GP0" Clk "Auto" }  [get_bd_intf_pins axi_gpio_0/S_AXI]


# Tell PS this is where AXI bus mappings start in memory
set_property offset 0x42000000 [get_bd_addr_segs {processing_system7_0/Data/SEG_axi_gpio_0_Reg}]
set_property range 4K [get_bd_addr_segs {processing_system7_0/Data/SEG_axi_gpio_0_Reg}]

# Generate system_wrapper.v
generate_target all [get_files  $bd_path/system.bd]
make_wrapper -files [get_files $bd_path/system.bd] -top
add_files -norecurse $bd_path/hdl/system_wrapper.v

set_property VERILOG_DEFINE {TOOL_VIVADO} [current_fileset]
set_property STRATEGY Flow_PerfOptimized_High [get_runs synth_1]
set_property STRATEGY Performance_NetDelay_high [get_runs impl_1]

########################## CONFIGURE GPIO ######################################################################

#Specify GPIO1 acts as input from FC to PS; GPIO2 acts as output from PS to FC
#1: make line read-only for CPU, 0 makes write-only
set_property -dict [list CONFIG.C_ALL_INPUTS {1} CONFIG.C_ALL_INPUTS_2 {0} CONFIG.C_ALL_OUTPUTS_2 {1}] [get_bd_cells axi_gpio_0]


#axi_gpio_0_i [31:0]
#axi_gpio_0_o [31:0]



############################ Export platform signals for RTL top ############################


# Export a known deasserted reset (make it explicit: CONST_VAL=1)
startgroup
create_bd_cell -type ip -vlnv xilinx.com:ip:xlconstant xlc_pdh_rst_n
set_property -dict [list CONFIG.CONST_VAL {1}] [get_bd_cells xlc_pdh_rst_n]
endgroup
create_bd_port -dir O pdh_rst_n
connect_bd_net [get_bd_ports pdh_rst_n] [get_bd_pins xlc_pdh_rst_n/dout]

# pdh_core axi_to_ps -> axi_gpio gpio_io_i (so BD port is input)
create_bd_port -dir I -from 31 -to 0 axi_to_ps
connect_bd_net [get_bd_ports axi_to_ps] [get_bd_pins axi_gpio_0/gpio_io_i]

# axi_gpio gpio2_io_o -> pdh_core axi_from_ps (so BD port is output)
create_bd_port -dir O -from 31 -to 0 axi_from_ps
connect_bd_net [get_bd_ports axi_from_ps] [get_bd_pins axi_gpio_0/gpio2_io_o]




############################## COMPILE NETLIST ########################################################################
# 1. Reset the BD targets (cleans out old, broken attempts)
reset_target all [get_files system.bd]

# 2. Set to Global Synthesis (This prevents the need for .dcp checkpoints)
set_property synth_checkpoint_mode None [get_files system.bd]

# 3. Generate the actual Verilog targets
generate_target all [get_files system.bd]

# 4. Final verification: make sure the wrapper is actually the top
set_property top pdh_top [current_fileset]

#Synthesis
synth_design -top pdh_top -flatten_hierarchy none -bufg 16 -keep_equivalent_registers
write_checkpoint         -force   $path_out/post_synth
report_timing_summary    -file    $path_out/post_synth_timing_summary.rpt
report_power             -file    $path_out/post_synth_power.rpt


#Placement + logic optimization
opt_design
power_opt_design
place_design
phys_opt_design
write_checkpoint         -force   $path_out/post_place
report_timing_summary    -file    $path_out/post_place_timing_summary.rpt


#Routing
route_design
write_checkpoint         -force   $path_out/post_route
report_timing_summary    -file    $path_out/post_route_timing_summary.rpt
report_timing            -file    $path_out/post_route_timing.rpt -sort_by group -max_paths 100 -path_type summary
report_clock_utilization -file    $path_out/clock_util.rpt
report_utilization       -file    $path_out/post_route_util.rpt
report_power             -file    $path_out/post_route_power.rpt
report_drc               -file    $path_out/post_imp_drc.rpt
report_io                -file    $path_out/post_imp_io.rpt
#write_verilog            -force   $path_out/bft_impl_netlist.v
#write_xdc -no_fixed_only -force   $path_out/bft_impl.xdc


#Verify IO config is reasonable
xilinx::ultrafast::report_io_reg -verbose -file $path_out/post_route_job.rpt

#Compressed bitstream flashes faster
set_property BITSTREAM.GENERAL.COMPRESS TRUE [current_design]

write_bitstream -force            $path_out/boot.bit


save_project_as -force $path_out/$prj_name.xpr


exit
