################################################################################
# minimal x86.mak for Red Pitaya Z10 / MASTER
################################################################################

CPU_CORES      ?= 4
BUILD_NUMBER   ?= 1
REVISION       ?= $(shell git rev-parse --short HEAD 2>/dev/null || echo local)
VERSION        ?= local
MODEL          ?= Z10
STREAMING      ?= MASTER

export BUILD_NUMBER
export REVISION
export VERSION

SUBMODULE_UBT  = "redpitaya-v2022.2"
SUBMODULE_LIN  = "branch-redpitaya-v2024.1"
SUBMODULE_APP  = ""

define GREET_MSG
##############################################################################
# Red Pitaya GNU/Linux Ecosystem
# U-Boot: $(SUBMODULE_UBT)
# Linux Kernel: $(SUBMODULE_LIN)
# Pro Applications: $(SUBMODULE_APP)
##############################################################################
endef
export GREET_MSG

################################################################################
# paths
################################################################################

DL        ?= $(CURDIR)/tmp/DL
TMP        = tmp

all: dtc_compiler devicetree u-boot linux boot

$(DL):
	mkdir -p $@

$(TMP):
	mkdir -p $@

$(BUILD_DIR):
	mkdir -p $@

################################################################################
# source tarballs (U-Boot, Linux, device-tree)
################################################################################

UBOOT_TAG       = redpitaya-v2022.2
LINUX_TAG       = branch-redpitaya-v2024.1
DTREE_TAG       = xilinx_v2022.1
DTREE_PATH_TAG  = xilinx-v2022.1

UBOOT_URL      ?= https://github.com/RedPitaya/u-boot-xlnx/archive/$(UBOOT_TAG).tar.gz
LINUX_URL      ?= https://github.com/RedPitaya/linux-xlnx/archive/$(LINUX_TAG).tar.gz
DTREE_URL      ?= https://github.com/Xilinx/device-tree-xlnx/archive/$(DTREE_TAG).tar.gz

UBOOT_DIR       = $(TMP)/u-boot-xlnx-$(UBOOT_TAG)
LINUX_DIR       = $(TMP)/linux-xlnx-$(LINUX_TAG)
DTREE_DIR       = $(TMP)/device-tree-xlnx-$(DTREE_PATH_TAG)

UBOOT_TAR       = $(DL)/u-boot-xlnx-$(UBOOT_TAG).tar.gz
LINUX_TAR       = $(DL)/linux-xlnx-$(LINUX_TAG).tar.gz
DTREE_TAR       = $(DL)/device-tree-xlnx-$(DTREE_TAG).tar.gz

LINUX_CFLAGS    = "-O2 -mtune=cortex-a9 -mfpu=neon -mfloat-abi=hard"
KERNEL_HEADERS ?= YES

################################################################################
# FPGA build: Z10 only (barebones project)
################################################################################

FPGA_COMMIT  ?= Release-2024.3
FPGA_UBOOT   ?= $(FPGA_COMMIT)
FPGA_MODEL   ?= Z10
FPGA_VERSION ?= z10_125

.PHONY: fpga

fpga: dtc_compiler $(BUILD_DIR)/fpga
	$(call build_fpga,$(PROJECT),$(FPGA_MODEL),$(DEF_PARAM),$(HWID))

define build_fpga
	mkdir -p $(BUILD_DIR)/fpga/$(FPGA_VERSION)/$(1)
	$(MAKE) -C $(RTL_DIR) PRJ=$(1) MODEL=$(2) DEFINES=$(3) HWID=$(4) DTS_VER=2022.1

	rm -rf fpga/$(FPGA_VERSION)/$(1)
	mkdir -p fpga/$(FPGA_VERSION)/$(1)
	mv fpga/build/* fpga/$(FPGA_VERSION)/$(1)

	cp    fpga/$(FPGA_VERSION)/$(1)/prj/$(1)/out/red_pitaya.bit       $(BUILD_DIR)/fpga/$(FPGA_VERSION)/$(1)/fpga.bit
	cp    fpga/$(FPGA_VERSION)/$(1)/prj/$(1)/sdk/fsbl/executable.elf  $(BUILD_DIR)/fpga/$(FPGA_VERSION)/$(1)/fsbl.elf
	cp -r fpga/$(FPGA_VERSION)/$(1)/prj/$(1)/sdk/dts                  $(BUILD_DIR)/fpga/$(FPGA_VERSION)/$(1)
	cp    fpga/$(FPGA_VERSION)/$(1)/prj/$(1)/dts/*                    $(BUILD_DIR)/fpga/$(FPGA_VERSION)/$(1)/dts/
	cp -f fpga/$(FPGA_VERSION)/$(1)/git_info.txt                      $(BUILD_DIR)/fpga/$(FPGA_VERSION)/$(1)/git_info.txt

	# fix overlay quirks
	sed -i 's/#address-cells/\/\/#address-cells/g' fpga/$(FPGA_VERSION)/$(1)/prj/$(1)/sdk/dts/pl.dtsi
	sed -i 's/#size-cells/\/\/#size-cells/g'      fpga/$(FPGA_VERSION)/$(1)/prj/$(1)/sdk/dts/pl.dtsi
	sed -i 's/red_pitaya.bit.bin/fpga.bit.bin/g'  fpga/$(FPGA_VERSION)/$(1)/prj/$(1)/sdk/dts/pl.dtsi
	grep -qxF '/include/ "pl_patch.dtsi"' fpga/$(FPGA_VERSION)/$(1)/prj/$(1)/sdk/dts/pl.dtsi || echo '/include/ "pl_patch.dtsi"' >> fpga/$(FPGA_VERSION)/$(1)/prj/$(1)/sdk/dts/pl.dtsi

	$(TMP)/dtc -@ -I dts -O dtb -o $(BUILD_DIR)/fpga/$(FPGA_VERSION)/$(1)/fpga.dtbo -i fpga/$(FPGA_VERSION)/$(1)/prj/$(1)/dts -i fpga/$(FPGA_VERSION)/$(1)/dts fpga/$(FPGA_VERSION)/$(1)/prj/$(1)/sdk/dts/pl.dtsi
	echo -n "all:{ $(BUILD_DIR)/fpga/$(FPGA_VERSION)/$(1)/fpga.bit }" > $(BUILD_DIR)/fpga/$(FPGA_VERSION)/$(1)/fpga.bif
	bootgen -image $(BUILD_DIR)/fpga/$(FPGA_VERSION)/$(1)/fpga.bif -arch zynq -process_bitstream bin -o $(BUILD_DIR)/fpga/$(FPGA_VERSION)/$(1)/red_pitaya.bit.bin -w
endef

$(BUILD_DIR)/fpga: $(DTREE_DIR)
	mkdir -p $(BUILD_DIR)/fpga

################################################################################
# device tree for main Linux
################################################################################

DEVICETREE     = devicetree.dtb
FPGA_DIRECTORY = $(patsubst $(BUILD_DIR)/fpga/%/,%, $(sort $(dir $(wildcard $(BUILD_DIR)/fpga/*/))))

.PHONY: devicetree devicetree-install

devicetree: devicetree-install dtc_compiler

$(DTREE_TAR): | $(DL)
	curl -L $(DTREE_URL) -o $@

$(DTREE_DIR): $(DTREE_TAR)
	mkdir -p $@
	tar -zxf $< --strip-components=1 --directory=$@
	mkdir -p fpga/tmp
	cp -r $@ fpga/tmp

$(FPGA_DIRECTORY): $(DTREE_DIR) $(TMP) $(BUILD_DIR)
	echo $@
	rm -rf $(TMP)/dts/$@
	mkdir -p $(TMP)/dts/$@/build
	cp -r $(BUILD_DIR)/fpga/$@/barebones/dts $(TMP)/dts/$@
	cat   $(TMP)/dts/$@/dts/system-top.dts > $(TMP)/dts/$@/build/system-top.dts
	cat   $(TMP)/dts/$@/dts/fpga.dts       >> $(TMP)/dts/$@/build/system-top.dts
	gcc -I $(TMP)/dts/$@/dts -E -nostdinc -undef -D__DTS__ -x assembler-with-cpp \
		-o $(TMP)/dts/$@/build/system-top.dts.tmp $(TMP)/dts/$@/build/system-top.dts
	$(TMP)/dtc -@ -I dts -O dtb -o $(TMP)/dts/$@/build/$(DEVICETREE) \
		-i $(TMP)/dts/$@/dts -i $(BUILD_DIR)/fpga/$@/barebones/dts_rp \
		$(TMP)/dts/$@/build/system-top.dts.tmp
	$(TMP)/dtc -I dtb -O dts --sort -o $(TMP)/dts/$@/build/dtraw.dts \
		$(TMP)/dts/$@/build/$(DEVICETREE)
	mkdir -p $(BUILD_DIR)/dts/$@
	cp -f $(TMP)/dts/$@/build/$(DEVICETREE) $(BUILD_DIR)/dts/$@
	cp -f $(TMP)/dts/$@/build/dtraw.dts     $(BUILD_DIR)/dts/$@

devicetree-install: $(FPGA_DIRECTORY)

################################################################################
# device tree for U-Boot
################################################################################

DEVICETREE_UB_PATH = fpga/uboot/dts_uboot
DEVICETREE_UB      = $(TMP)/devicetree_uboot.dtb

$(DEVICETREE_UB): dtc_compiler
	echo $@
	gcc -E -nostdinc -undef -D__DTS__ -x assembler-with-cpp \
		-o $(DEVICETREE_UB_PATH)/system-top.dts.tmp \
		   $(DEVICETREE_UB_PATH)/system-top.dts
	$(TMP)/dtc -@ -I dts -O dtb -o $(DEVICETREE_UB) \
		$(DEVICETREE_UB_PATH)/system-top.dts.tmp

################################################################################
# FSBL for U-Boot
################################################################################

FSBL = $(TMP)/fsbl.elf

.PHONY: fsbl_ub

fsbl_ub: $(FSBL)

$(FSBL): $(DEVICETREE_UB)
	echo $@
	$(MAKE) -C fpga/uboot PRJ=fsbl MODEL=Z10 DEFINES= HWID= DTS_VER=2022.1
	cp fpga/uboot/prj/fsbl/sdk/fsbl/executable.elf $@

################################################################################
# FSBL for 1 Gb RAM (Z20; kept for bootgen compatibility if you need it)
################################################################################

FSBL_1Gb_ram = $(TMP)/fsbl_1Gb_ram.elf

.PHONY: fsbl_ub_1Gb_ram

fsbl_ub_1Gb_ram: $(FSBL_1Gb_ram)

$(FSBL_1Gb_ram):
	echo $@
	./download_fpga.sh all fsbl_1Gb_ram $(GIT_MODE) $(FPGA_UBOOT) $(GITLAB_TOKEN) fsbl_1Gb_ram
	$(MAKE) -C fpga/fsbl_1Gb_ram PRJ=fsbl MODEL=Z20_250 DEFINES= HWID= DTS_VER=2022.1
	cp fpga/fsbl_1Gb_ram/prj/fsbl/sdk/fsbl/executable.elf $@

################################################################################
# U-Boot
################################################################################

UBOOT           = $(TMP)/u-boot.elf
UBOOT_SCRIPT        = $(BUILD_DIR)/u-boot.scr
UBOOT_SCRIPT_512Mb_ram = $(BUILD_DIR)/uboot/u-boot_512Mb_ram.scr
UBOOT_SCRIPT_1Gb_ram   = $(BUILD_DIR)/uboot/u-boot_1Gb_ram.scr

.PHONY: u-boot

u-boot: $(UBOOT) $(UBOOT_SCRIPT_512Mb_ram) $(UBOOT_SCRIPT_1Gb_ram)

$(UBOOT_TAR): | $(DL)
	curl -L $(UBOOT_URL) -o $@

$(UBOOT_DIR): $(UBOOT_TAR)
	rm -rf $@
	mkdir -p $@
	tar -zxf $< --strip-components=1 --directory=$@

$(UBOOT): $(UBOOT_DIR) $(TMP) $(DEVICETREE_UB)
	$(MAKE) -C $< distclean
	$(MAKE) -C $< clean
	$(MAKE) -C $< mrproper
	mkdir -p $</arch/arm/dts/
	cp $(DEVICETREE_UB) $</arch/arm/dts/red-pitaya.dtb
	$(MAKE) -C $< zynq_red_pitaya_defconfig -j $(CPU_CORES) DEVICE_TREE="red-pitaya"
	$(MAKE) -C $< -j $(CPU_CORES) DEVICE_TREE="red-pitaya"
	cp $(UBOOT_DIR)/u-boot.elf $@

$(UBOOT_SCRIPT_512Mb_ram): $(BUILD_DIR) $(UBOOT_DIR) $(UBOOT)
	mkdir -p $(BUILD_DIR)/uboot
	$(UBOOT_DIR)/tools/mkimage -A ARM -O linux -T script -C none -a 0 -e 0 \
		-n "boot Debian" -d patches/u-boot/u-boot.script $@
	cp $(UBOOT_SCRIPT_512Mb_ram) $(UBOOT_SCRIPT)

$(UBOOT_SCRIPT_1Gb_ram): $(UBOOT_SCRIPT_512Mb_ram)
	mkdir -p $(BUILD_DIR)/uboot
	$(UBOOT_DIR)/tools/mkimage -A ARM -O linux -T script -C none -a 0 -e 0 \
		-n "boot Debian" -d patches/u-boot/u-boot_1Gb_ram.script $@

################################################################################
# Linux kernel
################################################################################

LINUX = $(LINUX_DIR)/arch/arm/boot/uImage

.PHONY: linux linux_scripts dtc_compiler

linux: $(LINUX)

$(LINUX_TAR): | $(DL)
	curl -L $(LINUX_URL) -o $@

$(LINUX_DIR): $(LINUX_TAR)
	mkdir -p $@
	tar -zxf $< --strip-components=1 --directory=$@

linux_scripts: $(LINUX_DIR)
	$(MAKE) -C $(LINUX_DIR) clean
	$(MAKE) -C $(LINUX_DIR) mrproper
	$(MAKE) -C $(LINUX_DIR) ARCH=arm redpitaya_zynq_defconfig -j $(CPU_CORES)
	$(MAKE) -C $(LINUX_DIR) scripts

$(LINUX): linux_scripts
	$(MAKE) -C $(LINUX_DIR) clean
	$(MAKE) -C $(LINUX_DIR) mrproper
	$(MAKE) -C $(LINUX_DIR) ARCH=arm redpitaya_zynq_defconfig -j $(CPU_CORES)
	$(MAKE) -C $(LINUX_DIR) ARCH=arm C

::contentReference[oaicite:0]{index=0}
