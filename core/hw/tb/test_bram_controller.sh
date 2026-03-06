#!/usr/bin/env bash

#Run this from tb/

set -euo pipefail

TB=tb_bram_controller.sv
DUT=../rtl/bram_controller.sv
PD=../rtl/posedge_detector.sv
BRAM=../rtl/dc_bram.sv

rm -rf obj_dir dumps/tb_bram_controller.vcd sim 2>/dev/null || true

verilator -Wall -Wno-fatal \
  --trace-fst --trace-structs \
  -sv --binary \
  -o sim \
  "$TB" "$DUT" "$PD" "$BRAM"

./obj_dir/sim

echo "Waveform: wave.fst"
gtkwave wave.fst >/dev/null 2>&1 &

