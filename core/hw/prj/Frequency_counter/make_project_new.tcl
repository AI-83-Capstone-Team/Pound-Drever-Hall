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




################################################### Build cores ################################################

cd prj/$prj_name/cores
set core_names [glob -type d *]
cd ..

foreach core_name $core_names{
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



#################### ADD SOURCE AND CONSTRAINT FILES ###########################################################
add_files -norecurse rtl
set xdc_files [glob -nocomplain cfg/*.xdc]
if {[llength $xdc_files] > 0} {
  add_files -norecurse -fileset constrs_1 $xdc_files
}

#################### BUILD TOP-LEVEL WRAPPER ###################################################################
set project_name $prj_name















