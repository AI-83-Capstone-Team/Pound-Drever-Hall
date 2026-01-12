
#!/usr/bin/env bash

#Run this from tb/

set -euo pipefail

TB=tb_pdh_core.sv
DUT=../rtl/pdh_core.sv
PD=../rtl/posedge_detector.sv
SD=../rtl/state_change_detector.sv

rm -rf obj_dir wave.fst sim 2>/dev/null || true

verilator -Wall -Wno-fatal \
  --trace-fst --trace-structs \
  -sv --binary \
  -o sim \
  "$TB" "$DUT" "$PD"

./obj_dir/sim

echo "Waveform: wave.fst"
gtkwave wave.fst >/dev/null 2>&1 &

