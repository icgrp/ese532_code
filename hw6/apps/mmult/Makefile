#######################################################################################
.PHONY: help
help:
	@echo "Makefile Usage:"
	@echo ""
	@echo "  make all TARGET=<sw_emu|hw>"
	@echo "      Command to generate the design for specified Target and Device."
	@echo ""
	@echo "  make exe "
	@echo "      Command to generate host."
	@echo ""
	@echo "  make xclbin "
	@echo "      Command to generate hardware platform files(xo,xclbin)."
	@echo ""
	@echo "  make clean "
	@echo "      Command to remove the generated files."
	@echo ""
	@echo "  make afi EMAIL=<your email> "
	@echo "      Command to generate the fpga binary to run on AWS."
	@echo ""
#######################################################################################

# compiler tools
XILINX_VITIS ?= /media/lilbirb/research/Xilinx/Vitis/2020.1
XILINX_VIVADO ?= /media/lilbirb/research/Xilinx/Vivado/2020.1
XILINX_VIVADO_HLS ?= $(XILINX_VITIS)/Vivado_HLS

HOST_CXX ?= aarch64-linux-gnu-g++
VPP ?= ${XILINX_VITIS}/bin/v++
RM = rm -f
RMDIR = rm -rf

VITIS_PLATFORM = ese532_hw6_pfm
VITIS_PLATFORM_DIR ?= /media/lilbirb/research/git/avnet/petalinux/projects/ese532_hw6_pfm/export/ese532_hw6_pfm
VITIS_PLATFORM_PATH = $(VITIS_PLATFORM_DIR)/ese532_hw6_pfm.xpfm

# host compiler global settings
CXXFLAGS += -std=c++11 -DVITIS_PLATFORM=$(VITIS_PLATFORM) -D__USE_XOPEN2K8 -I$(XILINX_VIVADO)/include/ -I$(VITIS_PLATFORM_DIR)/sw/ese532_hw6_pfm/linux_domain/sysroot/aarch64-xilinx-linux/usr/include/xrt/ -O3 -g -Wall -c -fmessage-length=0 --sysroot=$(VITIS_PLATFORM_DIR)/sw/ese532_hw6_pfm/linux_domain/sysroot/aarch64-xilinx-linux
LDFLAGS += -lxilinxopencl -lpthread -lrt -ldl -lcrypt -lstdc++ -L$(VITIS_PLATFORM_DIR)/sw/ese532_hw6_pfm/linux_domain/sysroot/aarch64-xilinx-linux/usr/lib/ --sysroot=$(VITIS_PLATFORM_DIR)/sw/ese532_hw6_pfm/linux_domain/sysroot/aarch64-xilinx-linux

# hardware compiler shared settings
VPP_OPTS = --target hw

#
# OpenCL kernel files
#

BINARY_CONTAINERS += mmult.xclbin

BUILD_SUBDIRS += mmult.build
MMULT_OBJS += mmult.build/mmult.xo
ALL_KERNEL_OBJS += mmult.build/mmult.xo

ALL_MESSAGE_FILES = $(subst .xo,.mdb,$(ALL_KERNEL_OBJS)) $(subst .xclbin,.mdb,$(BINARY_CONTAINERS))

#
# host files
#

HOST_OBJECTS += src/vadd.o

HOST_EXE = host

BUILD_SUBDIRS += src/

#
# primary build targets
#

.PHONY: all clean
all: $(BINARY_CONTAINERS) $(HOST_EXE)

.NOTPARALLEL: clean

clean-host:
	-$(RM) $(HOST_EXE) $(HOST_OBJECTS)

clean-accelerators:
	-$(RM) $(BINARY_CONTAINERS) $(ALL_KERNEL_OBJS) $(ALL_MESSAGE_FILES)
	-$(RM) *.xclbin.sh *.xclbin.info *.xclbin.link_summary*
	-$(RMDIR) $(BUILD_SUBDIRS)
	-$(RMDIR) .Xil

clean-package:
	-${RMDIR} package
	-${RMDIR} package.build

clean: clean-host clean-accelerators clean-package

.PHONY: incremental
incremental: all


nothing:

#
# binary container: mmult.xclbin
#

mmult.build/krnl_vadd.xo: ../src/krnl_vadd.cpp mmult-krnl_vadd-compile.cfg common-config.cfg
	-@mkdir -p $(@D)
	-@$(RM) $@
	$(VPP) $(VPP_OPTS) --compile -I"$(<D)" --config common-config.cfg --config mmult-krnl_vadd-compile.cfg -o"$@" "$<"

mmult.xclbin: $(MMULT_OBJS) mmult-link.cfg common-config.cfg
	-@echo $(VPP) $(VPP_OPTS) --link --config common-config.cfg --config mmult-link.cfg -o"$@" $(MMULT_OBJS) > mmult.xclbin.sh
	$(VPP) $(VPP_OPTS) --link --config common-config.cfg --config mmult-link.cfg -o"$@" $(MMULT_OBJS)

#
# host rules
#

src/vadd.o: ../src/vadd.cpp ../src/vadd.h
	-@mkdir -p $(@D)
	$(HOST_CXX) $(CXXFLAGS) -o "$@" "$<"

$(HOST_EXE): $(HOST_OBJECTS)
	$(HOST_CXX) -o "$@" $(+) $(LDFLAGS)
	-@echo $(VPP) --package --config package.cfg  mmult.xclbin
	-@$(VPP) --package --config package.cfg  mmult.xclbin

.PHONE: cpu
cpu: $(CPU_EXE)

$(CPU_EXE): cpu/Host.cpp cpu/MMult.cpp ../common/EventTimer.cpp
	$(HOST_CXX) $(CXXFLAGS) -I./cpu -I../common $^ -o $@ $(LDFLAGS)
	


