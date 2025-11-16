vivado -mode tcl -source make_project.tcl
vivado -mode tcl -source synth.tcl
vivado -mode tcl -source imp.tcl
./format_bin.sh