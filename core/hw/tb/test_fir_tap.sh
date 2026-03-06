#!/usr/bin/env bash

# Run from tb/

set -euo pipefail

TB=tb_fir_tap.sv
DUT=../rtl/fir_tap.sv

mkdir -p dumps
rm -rf obj_dir dumps/tb_fir_tap.vcd 2>/dev/null || true

verilator -Wall -Wno-fatal \
  --trace --trace-structs \
  -sv --binary \
  -o sim \
  "$TB" "$DUT"

./obj_dir/sim

echo "Waveform: dumps/tb_fir_tap.vcd"
gtkwave dumps/tb_fir_tap.vcd >/dev/null 2>&1 &
