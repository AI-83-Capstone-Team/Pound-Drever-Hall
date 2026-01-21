

#!/usr/bin/env bash

#Run this from tb/

set -euo pipefail

TB=tb_dma_controller.sv
DUT=../rtl/dma_controller.sv
PD=../rtl/posedge_detector.sv

rm -rf obj_dir dumps/dma_tb.vcd sim 2>/dev/null || true

verilator -Wall -Wno-fatal \
  --trace-fst --trace-structs \
  -sv --binary \
  -o sim \
  "$TB" "$DUT" "$PD"

./obj_dir/sim

echo "Waveform: wave.fst"
gtkwave wave.fst >/dev/null 2>&1 &

