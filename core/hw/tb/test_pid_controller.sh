
#!/usr/bin/env bash

#Run this from tb/

set -euo pipefail

TB=tb_pid_controller.sv
DUT=../rtl/pid_core.sv
PD=../rtl/posedge_detector.sv

rm -rf obj_dir dumps/tb_pid_controller.vcd sim 2>/dev/null || true

verilator -Wall -Wno-fatal \
  --trace-fst --trace-structs \
  -sv --binary \
  -o sim \
  "$TB" "$DUT" "$PD"

./obj_dir/sim

echo "Waveform: tb_pid_controller.vcd"
gtkwave dumps/tb_pid_controller.vcd >/dev/null 2>&1 &

