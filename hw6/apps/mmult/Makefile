#######################################################################################
.PHONY: help
help:
	@echo "Makefile Usage:"
	@echo ""
	@echo "  make cpu"
	@echo "      Command to build mmult_cpu."
	@echo ""
	@echo "  make fpga "
	@echo "      Command to build mmult_fpga."
	@echo ""
	@echo "  make host "
	@echo "      Command to build only the host program of mmult_fpga."
	@echo ""
	@echo "  make clean "
	@echo "      Command to remove the generated files."
	@echo ""
#######################################################################################

HOST_CXX ?= aarch64-linux-gnu-g++
VPP = ${XILINX_VITIS}/bin/v++
RM = rm -f
RMDIR = rm -rf

VITIS_PLATFORM = u96v2_sbc_base
VITIS_PLATFORM_DIR = ${PLATFORM_REPO_PATHS}
VITIS_PLATFORM_PATH = $(VITIS_PLATFORM_DIR)/u96v2_sbc_base.xpfm

# host compiler global settings
CXXFLAGS += -march=armv8-a+simd -mtune=cortex-a53 -std=c++11 -DVITIS_PLATFORM=$(VITIS_PLATFORM) -D__USE_XOPEN2K8 -I$(XILINX_VIVADO)/include/ -I$(VITIS_PLATFORM_DIR)/sw/$(VITIS_PLATFORM)/PetaLinux/sysroot/aarch64-xilinx-linux/usr/include/xrt/ -O3 -g -Wall -c -fmessage-length=0 --sysroot=$(VITIS_PLATFORM_DIR)/sw/$(VITIS_PLATFORM)/PetaLinux/sysroot/aarch64-xilinx-linux
LDFLAGS += -lxilinxopencl -lpthread -lrt -ldl -lcrypt -lstdc++ -L$(VITIS_PLATFORM_DIR)/sw/$(VITIS_PLATFORM)/PetaLinux/sysroot/aarch64-xilinx-linux/usr/lib/ --sysroot=$(VITIS_PLATFORM_DIR)/sw/$(VITIS_PLATFORM)/PetaLinux/sysroot/aarch64-xilinx-linux

# hardware compiler shared settings
VPP_OPTS = --target hw


#
# OpenCL kernel files
#
XO := kernel.xo
XCLBIN := kernel.xclbin
ALL_MESSAGE_FILES = $(subst .xo,.mdb,$(XO)) $(subst .xclbin,.mdb,$(XCLBIN))

#
# host files
#
HOST_SOURCES = fpga/Host.cpp ../../common/EventTimer.cpp ../../common/Utilities.cpp
HOST_OBJECTS =$(HOST_SOURCES:.cpp=.o)
HOST_EXE = host

CPU_SOURCES = cpu/Host.cpp ../../common/EventTimer.cpp ../../common/Utilities.cpp
CPU_OBJECTS=$(CPU_SOURCES:.cpp=.o)
CPU_EXE = mmult_cpu

.PHONY: cpu
cpu: $(CPU_EXE)

$(CPU_EXE): $(CPU_OBJECTS)
	$(HOST_CXX) -I./fpga/hls/ -o "$@" $(+) $(LDFLAGS)

$(HOST_EXE): $(HOST_OBJECTS)
	$(HOST_CXX) -o "$@" $(+) $(LDFLAGS)

.cpp.o:
	$(HOST_CXX) $(CXXFLAGS) -I./fpga/hls/ -I../../common -o "$@" "$<"

#
# primary build targets
#

.PHONY: fpga clean
fpga: package/sd_card.img

.NOTPARALLEL: clean

clean-cpu:
	-$(RM) $(CPU_EXE) $(CPU_OBJECTS) 

clean-host:
	-$(RM) $(HOST_EXE) $(HOST_OBJECTS) 

clean-accelerators:
	-$(RM) $(XCLBIN) $(XO) $(ALL_MESSAGE_FILES)
	-$(RM) *.xclbin.sh *.xclbin.info *.xclbin.link_summary* *.compile_summary
	-$(RMDIR) .Xil fpga/hls/proj_mmult

clean-package:
	-${RMDIR} package
	-${RMDIR} package.build

clean: clean-cpu clean-host clean-accelerators clean-package
	-$(RM) *.log *.package_summary
	-${RMDIR} _x .ipcache

#
# binary container: kernel.xclbin
#

$(XO): fpga/hls/MMult.cpp
	-@$(RM) $@
	$(VPP) $(VPP_OPTS) -k mmult_fpga --compile -I"$(<D)" --config fpga/design.cfg -o"$@" "$<"

$(XCLBIN): $(XO)
	$(VPP) $(VPP_OPTS) --link --config fpga/design.cfg -o"$@" $(+)

package/sd_card.img: $(HOST_EXE) $(XCLBIN) ./fpga/xrt.ini
	$(VPP) --package $(VPP_OPTS) --config fpga/package.cfg $(XCLBIN) \
		--package.out_dir package \
		--package.sd_file $(HOST_EXE) \
		--package.kernel_image $(PLATFORM_REPO_PATHS)/sw/$(VITIS_PLATFORM)/PetaLinux/image/image.ub \
		--package.rootfs $(PLATFORM_REPO_PATHS)/sw/$(VITIS_PLATFORM)/PetaLinux/rootfs/rootfs.ext4 \
		--package.sd_file $(XCLBIN) \
		--package.sd_file ./fpga/xrt.ini
