PRJ   = pdh_core
HWID ?= ""
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
	rm -rf prj/$(PRJ)/out prj/$(PRJ)/.Xil prj/$(PRJ)/.srcs prj/$(PRJ)/sdk prj/$(PRJ)/project prj/$(PRJ)/.hbs prj/$(PRJ)/*.log prj/$(PRJ)/usage_statistics*
	rm -f *.txt
	rm -f *.log

sim: 
	$(VIVADO) -source red_pitaya_vivado_sim.tcl -tclargs $(PRJ) Z10 $(DEFINES)

$(FPGA_BIT):
ifneq ($(HWID),"")
	$(VIVADO) -source red_pitaya_vivado_Z10.tcl -tclargs $(PRJ) $(DEFINES) HWID=$(HWID)
else
	$(VIVADO) -source red_pitaya_vivado_Z10.tcl -tclargs $(PRJ) $(DEFINES)
endif
	./synCheck.sh

# $(FSBL_ELF): $(FPGA_BIT)
# 	xsct red_pitaya_hsi_fsbl.tcl $(PRJ)
# 
# $(DEVICE_TREE): $(FPGA_BIT)
# 	xsct red_pitaya_hsi_dts.tcl  $(PRJ) DTS_VER=$(DTS_VER)

$(FPGA_BIN): $(FPGA_BIT)
	@echo all:{$(FPGA_BIT)} > build/boot.bif
	bootgen -image build/boot.bif -arch zynq -process_bitstream bin
	mv build/boot.bit.bin build/boot.bin 
