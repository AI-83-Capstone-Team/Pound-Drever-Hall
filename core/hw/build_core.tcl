################################# PDH CORE BUILD SCRIPT ####################

set prj_name [lindex $argv 0]
set prj_defs [lindex $argv 1]
puts "Project name: $prj_name"
puts "Defines: $prj_defs"
cd prj/$prj_name

tclapp::install -quiet ultrafast

#TODO: Move IP path outside proj dir
set path_brd ../../brd
set path_rtl ../../rtl
set path_ip ip
set path_sdc sdc
set path_out ../../build

set path_sdk sdk
set path_bd  .srcs/sources_1/bd/system/hdl

file mkdir $path_out
file mkdir $path_sdk

set_param board.repoPaths [list $path_brd]
set_param iconstr.diffPairPulltype {opposite}

#Create project and pass config args
set part xc7z010clg400-1
create_project -in_memory -part $part
set_property verilog_define $prj_defs [current_fileset]


#Add HPS IP block
#TODO Verify this width is correct
set ::gpio_width 24
source $path_ip/systemZ10.tcl


#Generate SDK files (can probably get rid of this)
generate_target all [get_files    system.bd]
write_hwdef -force       -file    $path_sdk/red_pitaya.hwdef

add_files -quiet                  [glob -nocomplain       $path_rtl/*_pkg.sv]

#RP RTL Library
add_files $path_rtl

#Propject RTL
add_files rtl

#Physical
add_files $path_bd

#Project-specific ip cores
set ip_files [glob -nocomplain $path_ip/*.xci]
if {$ip_files != ""} {
add_files                         $ip_files
}

#Design constraints
add_files -fileset constrs_1      $path_sdc/red_pitaya.xdc



#Synthesis
synth_design -top red_pitaya_top -flatten_hierarchy none -bufg 16 -keep_equivalent_registers
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
#write_hwdef              -file    $path_sdk/red_pitaya.hwdef


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

write_sysdef -force      -hwdef   $path_sdk/red_pitaya.hwdef \
                         -bitfile $path_out/boot.bit \
                         -file    $path_sdk/red_pitaya.sysdef

exit
