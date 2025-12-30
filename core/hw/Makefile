PROJECTS   = $(notdir $(wildcard prj/*)) 
DEFINES ?= ""
DTS_VER = DTS_VER=2022.1

# build artifacts
FPGA_BIT    = build/boot.bit
FPGA_BIN    = build/boot.bin
FSBL_ELF    = build/fsbl/fsbl.elf
MEMTEST_ELF = build/dram_test_executable.elf
DEVICE_TREE = build/system.dts

VIVADO = vivado -nojournal -mode batch
HSI    = hsi    -nolog -nojournal -mode batch
#HSI    = hsi    -nolog -mode batch

.PHONY: all clean $(PROJECTS)

all:
	@echo "Specify a project: $(PROJECTS)"


$(PROJECTS):
	@mkdir -p build/$@
	$(VIVADO) -source prj/$@/make_project.tcl -tclargs $@ $(DEFINES)
	# Post-processing
	@echo "all:{build/boot.bit}" > build/boot.bif
	bootgen -image build/boot.bif -arch zynq -process_bitstream bin
	@mv vivado.log build/vivado.log



clean:
	rm -rf build
	rm -rf prj/*/{out,.Xil,.srcs,sdk,.hbs,*.log,usage_statistics*}  
	rm -f *.txt *.log vivado.jou vivado_pid*.str


$(FPGA_BIT):
	$(VIVADO) -source prj/$(PRJ)/make_project.tcl -tclargs $(PRJ) $(DEFINES)
	./synCheck.sh


$(FPGA_BIN): $(FPGA_BIT)
	@echo all:{$(FPGA_BIT)} > build/boot.bif
	bootgen -image build/boot.bif -arch zynq -process_bitstream bin
	mv build/boot.bit.bin build/boot.bin
