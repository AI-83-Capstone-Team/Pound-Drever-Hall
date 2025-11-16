echo -n "all: { tmp/freq/freq.runs/impl_1/system_wrapper.bit }" > tmp/freq/freq.runs/impl_1/system_wrapper.bif
bootgen -image tmp/freq/freq.runs/impl_1/system_wrapper.bif -arch zynq -process_bitstream bin -o system_wrapper.bit.bin -w
cp tmp/freq/freq.runs/impl_1/system_wrapper.bit.bin build/system_wrapper.bit.bin