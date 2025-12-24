PRJ   = pdh_core
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

.PHONY: all clean sim

all: $(FPGA_BIN) 

clean:
	rm -rf build
	rm -rf out .Xil .srcs sdk sim
	rm -rf pdh_core/out pdh_core/.Xil pdh_core/.srcs pdh_core/sdk /pdh_core/project pdh_core/.hbs pdh_core/*.log pdh_core/usage_statistics*
	rm -f *.txt
	rm -f *.log

sim: 
	$(VIVADO) -source red_pitaya_vivado_sim.tcl -tclargs pdh_core Z10 $(DEFINES)

$(FPGA_BIT):
	$(VIVADO) -source red_pitaya_vivado_Z10.tcl -tclargs pdh_core $(DEFINES)
	./synCheck.sh

# $(FSBL_ELF): $(FPGA_BIT)
# 	xsct red_pitaya_hsi_fsbl.tcl pdh_core
# 
# $(DEVICE_TREE): $(FPGA_BIT)
# 	xsct red_pitaya_hsi_dts.tcl  pdh_core DTS_VER=$(DTS_VER)

$(FPGA_BIN): $(FPGA_BIT)
	@echo all:{$(FPGA_BIT)} > build/boot.bif
	bootgen -image build/boot.bif -arch zynq -process_bitstream bin
	mv build/boot.bit.bin build/boot.bin 
