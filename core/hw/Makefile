DEFINES ?= ""

VIVADO = vivado -nojournal -mode batch

.PHONY: all clean pdh_core

all:
	@echo "Targets: pdh_core  clean"

pdh_core:
	@mkdir -p build
	$(VIVADO) -source make_project.tcl -tclargs pdh_core $(DEFINES)
	@echo "all:{build/boot.bit}" > build/boot.bif
	bootgen -image build/boot.bif -arch zynq -process_bitstream bin
	@mv build/boot.bit.bin build/boot.bin
	@mv vivado.log build/vivado.log

clean:
	rm -rf build tmp .Xil
	rm -rf sim/build sim/__pycache__ sim/results.xml sim/sine_qtr_4096_16b.mem
	rm -f *.txt *.log vivado.jou vivado_pid*.str
