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
add_files -norecurse -fileset constrs_1 cfg/ports.xdc
add_files -norecurse -fileset constrs_1 cfg/clocks.xdc




################### ADD PORTS ##################################################################################

### ADC
create_bd_port -dir I -from 13 -to 0 adc_dat_a_i
create_bd_port -dir I -from 13 -to 0 adc_dat_b_i
create_bd_port -dir I adc_clk_p_i
create_bd_port -dir I adc_clk_n_i
create_bd_port -dir O adc_enc_p_o
create_bd_port -dir O adc_enc_n_o
create_bd_port -dir O adc_csn_o


### DAC
create_bd_port -dir O -from 13 -to 0 dac_dat_o
create_bd_port -dir O dac_clk_o
create_bd_port -dir O dac_rst_o
create_bd_port -dir O dac_sel_o
create_bd_port -dir O dac_wrt_o


### PWM
create_bd_port -dir O -from 3 -to 0 dac_pwm_o


### XADC
create_bd_intf_port -mode Slave -vlnv xilinx.com:interface:diff_analog_io_rtl:1.0 Vp_Vn
create_bd_intf_port -mode Slave -vlnv xilinx.com:interface:diff_analog_io_rtl:1.0 Vaux0
create_bd_intf_port -mode Slave -vlnv xilinx.com:interface:diff_analog_io_rtl:1.0 Vaux1
create_bd_intf_port -mode Slave -vlnv xilinx.com:interface:diff_analog_io_rtl:1.0 Vaux9
create_bd_intf_port -mode Slave -vlnv xilinx.com:interface:diff_analog_io_rtl:1.0 Vaux8


### Expansion connector
create_bd_port -dir IO -from 7 -to 0 exp_p_tri_io
create_bd_port -dir IO -from 7 -to 0 exp_n_tri_io


### SATA connector
create_bd_port -dir O -from 1 -to 0 daisy_p_o
create_bd_port -dir O -from 1 -to 0 daisy_n_o

create_bd_port -dir I -from 1 -to 0 daisy_p_i
create_bd_port -dir I -from 1 -to 0 daisy_n_i


### LED
create_bd_port -dir O -from 7 -to 0 led_o





################### CONNECT CORES #########################################################################
set_property IP_REPO_PATHS tmp/cores [current_project]
update_ip_catalog

# Zynq processing system with RedPitaya specific preset
startgroup
create_bd_cell -type ip -vlnv xilinx.com:ip:processing_system7 processing_system7_0
set_property -dict [list CONFIG.PCW_USE_S_AXI_HP0 {1}] [get_bd_cells processing_system7_0]
set_property -dict [list CONFIG.PCW_IMPORT_BOARD_PRESET {cfg/red_pitaya.xml}] [get_bd_cells processing_system7_0]
endgroup


# Buffers for differential IOs - Daisychain
startgroup
create_bd_cell -type ip -vlnv xilinx.com:ip:util_ds_buf util_ds_buf_1
set_property -dict [list CONFIG.C_SIZE {2}] [get_bd_cells util_ds_buf_1]

create_bd_cell -type ip -vlnv xilinx.com:ip:util_ds_buf util_ds_buf_2
set_property -dict [list CONFIG.C_SIZE {2}] [get_bd_cells util_ds_buf_2]
set_property -dict [list CONFIG.C_BUF_TYPE {OBUFDS}] [get_bd_cells util_ds_buf_2]
endgroup


# AXI GPIO IP core
startgroup
create_bd_cell -type ip -vlnv xilinx.com:ip:axi_gpio axi_gpio_0
set_property -dict [list CONFIG.C_IS_DUAL {1} CONFIG.C_ALL_INPUTS_2 {1}] [get_bd_cells axi_gpio_0]
endgroup


#ADC AXI protocol adapter, convert from 14 bit raw parallel to AXI
startgroup
create_bd_cell -type ip -vlnv pavel-demin:user:axis_red_pitaya_adc axis_red_pitaya_adc_0
endgroup


#DAC AXI protocol adapter
startgroup
create_bd_cell -type ip -vlnv pavel-demin:user:axis_red_pitaya_dac axis_red_pitaya_dac_0
endgroup


#Clocking Wizard
startgroup
create_bd_cell -type ip -vlnv xilinx.com:ip:clk_wiz clk_wiz_0
set_property -dict [list CONFIG.PRIM_IN_FREQ.VALUE_SRC USER] [get_bd_cells clk_wiz_0]
set_property -dict [list CONFIG.PRIM_IN_FREQ {125.000} CONFIG.CLKOUT1_REQUESTED_OUT_FREQ {250.000} CONFIG.USE_RESET {false} CONFIG.CLKIN1_JITTER_PS {80.0} CONFIG.MMCM_DIVCLK_DIVIDE {1} CONFIG.MMCM_CLKFBOUT_MULT_F {8.000} CONFIG.MMCM_CLKIN1_PERIOD {8.0} CONFIG.MMCM_CLKOUT0_DIVIDE_F {4.000} CONFIG.CLKOUT1_JITTER {104.759} CONFIG.CLKOUT1_PHASE_ERROR {96.948}] [get_bd_cells clk_wiz_0]
endgroup


#DDS
startgroup
create_bd_cell -type ip -vlnv xilinx.com:ip:dds_compiler dds_compiler_0
set_property -dict [list CONFIG.PartsPresent {Phase_Generator_and_SIN_COS_LUT} CONFIG.Parameter_Entry {System_Parameters} CONFIG.Spurious_Free_Dynamic_Range {84} CONFIG.Frequency_Resolution {0.5} CONFIG.Amplitude_Mode {Unit_Circle} CONFIG.DDS_Clock_Rate {125} CONFIG.Noise_Shaping {Auto} CONFIG.Phase_Width {28} CONFIG.Output_Width {14} CONFIG.Has_Phase_Out {false} CONFIG.DATA_Has_TLAST {Not_Required} CONFIG.S_PHASE_Has_TUSER {Not_Required} CONFIG.M_DATA_Has_TUSER {Not_Required} CONFIG.Latency {8} CONFIG.Output_Frequency1 {3.90625} CONFIG.PINC1 {0}] [get_bd_cells dds_compiler_0]
endgroup

#Configure DDS
set_property -dict [list CONFIG.Parameter_Entry {System_Parameters} CONFIG.Phase_Increment {Streaming} CONFIG.Phase_offset {None} CONFIG.Amplitude_Mode {Full_Range} CONFIG.Has_Phase_Out {false} CONFIG.S_PHASE_Has_TUSER {Not_Required} CONFIG.Latency_Configuration {Auto} CONFIG.Frequency_Resolution {0.5} CONFIG.Noise_Shaping {Auto} CONFIG.Phase_Width {28} CONFIG.Output_Width {14} CONFIG.DATA_Has_TLAST {Not_Required} CONFIG.S_PHASE_Has_TUSER {Not_Required} CONFIG.M_DATA_Has_TUSER {Not_Re/xlquired} CONFIG.M_PHASE_Has_TUSER {Not_Required} CONFIG.Latency {8} CONFIG.Output_Frequency1 {0} CONFIG.PINC1 {0}] [get_bd_cells dds_compiler_0]




#################### BUILD TOP-LEVEL WRAPPER AND SPECIFY LINK TO PS #####################################################

#This daisy chain stuff seems like a ghost circuit atm
connect_bd_net [get_bd_ports daisy_p_i] [get_bd_pins util_ds_buf_1/IBUF_DS_P]
connect_bd_net [get_bd_ports daisy_n_i] [get_bd_pins util_ds_buf_1/IBUF_DS_N]
connect_bd_net [get_bd_ports daisy_p_o] [get_bd_pins util_ds_buf_2/OBUF_DS_P]
connect_bd_net [get_bd_ports daisy_n_o] [get_bd_pins util_ds_buf_2/OBUF_DS_N]
connect_bd_net [get_bd_pins util_ds_buf_1/IBUF_OUT] [get_bd_pins util_ds_buf_2/OBUF_IN]



#Expose asic-level signals to the FPGA netlist
apply_bd_automation -rule xilinx.com:bd_rule:processing_system7 -config {make_external "FIXED_IO, DDR" Master "Disable" Slave "Disable" }  [get_bd_cells processing_system7_0]

#Clock master and slave AXI FSM's via PS clk
connect_bd_net [get_bd_pins processing_system7_0/M_AXI_GP0_ACLK] [get_bd_pins processing_system7_0/FCLK_CLK0]
connect_bd_net [get_bd_pins processing_system7_0/S_AXI_HP0_ACLK] [get_bd_pins processing_system7_0/FCLK_CLK0]



# NOTE: Physical assignments for ports are in ports.xdc
# ADC AXI Wrapper
connect_bd_net [get_bd_ports adc_clk_p_i] [get_bd_pins axis_red_pitaya_adc_0/adc_clk_p]
connect_bd_net [get_bd_ports adc_clk_n_i] [get_bd_pins axis_red_pitaya_adc_0/adc_clk_n]
connect_bd_net [get_bd_ports adc_dat_a_i] [get_bd_pins axis_red_pitaya_adc_0/adc_dat_a]
connect_bd_net [get_bd_ports adc_dat_b_i] [get_bd_pins axis_red_pitaya_adc_0/adc_dat_b]
connect_bd_net [get_bd_ports adc_csn_o] [get_bd_pins axis_red_pitaya_adc_0/adc_csn]


# DAC AXI Wrapper
connect_bd_net [get_bd_ports dac_clk_o] [get_bd_pins axis_red_pitaya_dac_0/dac_clk]
connect_bd_net [get_bd_ports dac_rst_o] [get_bd_pins axis_red_pitaya_dac_0/dac_rst]
connect_bd_net [get_bd_ports dac_sel_o] [get_bd_pins axis_red_pitaya_dac_0/dac_sel]
connect_bd_net [get_bd_ports dac_wrt_o] [get_bd_pins axis_red_pitaya_dac_0/dac_wrt]
connect_bd_net [get_bd_ports dac_dat_o] [get_bd_pins axis_red_pitaya_dac_0/dac_dat]
connect_bd_net [get_bd_pins clk_wiz_0/locked] [get_bd_pins axis_red_pitaya_dac_0/locked]
connect_bd_net [get_bd_pins clk_wiz_0/clk_out1] [get_bd_pins axis_red_pitaya_dac_0/ddr_clk]
connect_bd_net [get_bd_pins axis_red_pitaya_dac_0/aclk] [get_bd_pins axis_red_pitaya_adc_0/adc_clk]
connect_bd_intf_net [get_bd_intf_pins dds_compiler_0/M_AXIS_DATA] [get_bd_intf_pins axis_red_pitaya_dac_0/S_AXIS]
connect_bd_net [get_bd_pins clk_wiz_0/clk_in1] [get_bd_pins dds_compiler_0/aclk]
connect_bd_net [get_bd_pins dds_compiler_0/aclk] [get_bd_pins axis_red_pitaya_adc_0/adc_clk]


# Using the AXI GPIO logic to route signals between our FC RTL and the PS
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
set_property -dict [list CONFIG.C_ALL_INPUTS {1} CONFIG.C_ALL_INPUTS_2 {0}] [get_bd_cells axi_gpio_0]


#Split the 32-bit GPIO2 output signal into a 5-bit log2Ncycles signal and a 27-bit phase signal
startgroup
create_bd_cell -type ip -vlnv xilinx.com:ip:xlslice xls_log2Ncycles
set_property -dict [list CONFIG.DIN_TO {0} CONFIG.DIN_FROM {4}] [get_bd_cells xls_log2Ncycles]
create_bd_cell -type ip -vlnv xilinx.com:ip:xlslice xls_phase
set_property -dict [list CONFIG.DIN_TO {5} CONFIG.DIN_FROM {31}] [get_bd_cells xls_phase]
endgroup




############################ User-Written Modules  #####################################################################
#Create reset signal
startgroup
create_bd_cell -type ip -vlnv xilinx.com:ip:xlconstant xlc_reset
endgroup

# AXIS Constant from Pavel Demin
create_bd_cell -type ip -vlnv pavel-demin:user:axis_constant axis_constant_0

# signal split
create_bd_cell -type module -reference signal_split signal_split_0

# pow2 module
create_bd_cell -type module -reference pow2 pos2_0

# signal_decoder
create_bd_cell -type module -reference signal_decoder signal_decoder_0

# frequency_coutner
create_bd_cell -type module -reference frequency_counter frequency_counter_0



############################# Wire RTL Modules #########################################################


# signal_split
connect_bd_intf_net [get_bd_intf_pins signal_split_0/S_AXIS] [get_bd_intf_pins axis_red_pitaya_adc_0/M_AXIS]

# signal_decoder
connect_bd_intf_net [get_bd_intf_pins frequency_counter_0/M_AXIS_OUT] [get_bd_intf_pins signal_decoder_0/S_AXIS]
connect_bd_net [get_bd_pins signal_decoder_0/clk] [get_bd_pins axis_red_pitaya_adc_0/adc_clk]
connect_bd_net [get_bd_pins xlc_reset/dout] [get_bd_pins signal_decoder_0/rst]
connect_bd_net [get_bd_ports led_o] [get_bd_pins signal_decoder_0/led_out]

# frequency_counter
connect_bd_intf_net [get_bd_intf_pins frequency_counter_0/S_AXIS_IN] [get_bd_intf_pins signal_split_0/M_AXIS_PORT1]
connect_bd_net [get_bd_pins frequency_counter_0/clk] [get_bd_pins axis_red_pitaya_adc_0/adc_clk]
connect_bd_net [get_bd_pins frequency_counter_0/rst] [get_bd_pins xlc_reset/dout]
connect_bd_net [get_bd_pins pos2_0/N] [get_bd_pins frequency_counter_0/Ncycles]
connect_bd_net [get_bd_pins xls_log2Ncycles/Dout] [get_bd_pins pos2_0/log2N]

# to GPIO
connect_bd_net [get_bd_pins frequency_counter_0/counter_output] [get_bd_pins axi_gpio_0/gpio_io_i]
connect_bd_net [get_bd_pins axi_gpio_0/gpio2_io_i] [get_bd_pins axi_gpio_0/gpio2_io_o]
connect_bd_net [get_bd_pins xls_log2Ncycles/Din] [get_bd_pins axi_gpio_0/gpio2_io_o]


# DDS compiler
connect_bd_intf_net [get_bd_intf_pins axis_constant_0/M_AXIS] [get_bd_intf_pins dds_compiler_0/S_AXIS_PHASE]


# AXIS Constant
connect_bd_net [get_bd_pins axis_red_pitaya_adc_0/adc_clk] [get_bd_pins axis_constant_0/aclk]
connect_bd_net [get_bd_pins xls_phase/Dout] [get_bd_pins axis_constant_0/cfg_data]
connect_bd_net [get_bd_pins xls_phase/Din] [get_bd_pins axi_gpio_0/gpio2_io_o]




############################### PACKAGING #########################################################################
group_bd_cells SignalGenerator [get_bd_cells axis_red_pitaya_dac_0] [get_bd_cells dds_compiler_0] [get_bd_cells clk_wiz_0] [get_bd_cells axis_constant_0] [get_bd_cells xls_phase]

group_bd_cells DataAcquisition [get_bd_cells axis_red_pitaya_adc_0] [get_bd_cells signal_split_0]

group_bd_cells FrequencyCounter [get_bd_cells xls_log2Ncycles] [get_bd_cells pos2_0] [get_bd_cells frequency_counter_0]

group_bd_cells PS7 [get_bd_cells processing_system7_0] [get_bd_cells rst_ps7_0_125M] [get_bd_cells ps7_0_axi_periph]




############################## COMPILE NETLIST ########################################################################
# 1. Reset the BD targets (cleans out old, broken attempts)
reset_target all [get_files system.bd]

# 2. Set to Global Synthesis (This prevents the need for .dcp checkpoints)
set_property synth_checkpoint_mode None [get_files system.bd]

# 3. Generate the actual Verilog targets
generate_target all [get_files system.bd]

# 4. Final verification: make sure the wrapper is actually the top
set_property top system_wrapper [current_fileset]


#Synthesis
synth_design -top system_wrapper -flatten_hierarchy none -bufg 16 -keep_equivalent_registers
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
