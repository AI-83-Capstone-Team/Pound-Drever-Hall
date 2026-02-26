
#!/usr/bin/env bash

#Run this from tb/

set -euo pipefail

TB=tb_nco.sv
DUT=../rtl/nco.sv
QTR=../rtl/sine_qtr_rom.sv
MEM=../rtl/sine_qtr_4096_16b.mem

cp -f "$MEM" .
rm -rf obj_dir dumps/tb_nco.vcd sim 2>/dev/null || true

verilator -DNOSYNTH -Wall -Wno-fatal \
  --trace-fst --trace-structs \
  -sv --binary \
  -o sim \
  "$TB" "$DUT" "$QTR" 

./obj_dir/sim

echo "Waveform: wave.fst"
gtkwave wave.fst >/dev/null 2>&1 &

