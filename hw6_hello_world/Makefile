# compiler tools
XILINX_VITIS ?= /media/lilbirb/research/Xilinx/Vitis/2020.1
XILINX_VIVADO ?= /media/lilbirb/research/Xilinx/Vivado/2020.1
XILINX_VIVADO_HLS ?= $(XILINX_VITIS)/Vivado_HLS

HOST_CXX ?= aarch64-linux-gnu-g++
VPP ?= ${XILINX_VITIS}/bin/v++
RM = rm -f
RMDIR = rm -rf

VITIS_PLATFORM = ese532_hw6_pfm
VITIS_PLATFORM_DIR = ${PLATFORM_REPO_PATHS}
VITIS_PLATFORM_PATH = $(VITIS_PLATFORM_DIR)/ese532_ultra96_pfm.xpfm

# host compiler global settings
CXXFLAGS += -march=armv8-a+simd -mtune=cortex-a53 -std=c++11 -DVITIS_PLATFORM=$(VITIS_PLATFORM) -D__USE_XOPEN2K8 -I$(XILINX_VIVADO)/include/ -I$(VITIS_PLATFORM_DIR)/sw/ese532_hw6_pfm/linux_domain/sysroot/aarch64-xilinx-linux/usr/include/xrt/ -O3 -g -Wall -c -fmessage-length=0 --sysroot=$(VITIS_PLATFORM_DIR)/sw/ese532_hw6_pfm/linux_domain/sysroot/aarch64-xilinx-linux
LDFLAGS += -lxilinxopencl -lpthread -lrt -ldl -lcrypt -lstdc++ -L$(VITIS_PLATFORM_DIR)/sw/ese532_hw6_pfm/linux_domain/sysroot/aarch64-xilinx-linux/usr/lib/ --sysroot=$(VITIS_PLATFORM_DIR)/sw/ese532_hw6_pfm/linux_domain/sysroot/aarch64-xilinx-linux

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
HOST_SOURCES = vadd.cpp
HOST_OBJECTS=$(HOST_SOURCES:.cpp=.o)
HOST_EXE = vadd

.PHONY: host

host: $(HOST_EXE)

$(HOST_EXE): $(HOST_OBJECTS)
	$(HOST_CXX) -o "$@" $(+) $(LDFLAGS)
	-@echo $(VPP) --package --config package.cfg --package.kernel_image $(PLATFORM_REPO_PATHS)/sw/ese532_hw6_pfm/linux_domain/image/image.ub --package.rootfs $(PLATFORM_REPO_PATHS)/sw/ese532_hw6_pfm/linux_domain/rootfs/rootfs.ext4 $(XCLBIN)
	-@$(VPP) --package --config package.cfg --package.kernel_image $(PLATFORM_REPO_PATHS)/sw/ese532_hw6_pfm/linux_domain/image/image.ub --package.rootfs $(PLATFORM_REPO_PATHS)/sw/ese532_hw6_pfm/linux_domain/rootfs/rootfs.ext4 $(XCLBIN)
		
.cpp.o:
	$(HOST_CXX) $(CXXFLAGS) -o "$@" "$<"

#
# primary build targets
#

.PHONY: fpga clean
fpga: $(XCLBIN) $(HOST_EXE)

.NOTPARALLEL: clean

clean-host:
	-$(RM) $(HOST_EXE) $(HOST_OBJECTS) 

clean-accelerators:
	-$(RM) $(XCLBIN) $(XO) $(ALL_MESSAGE_FILES)
	-$(RM) *.xclbin.sh *.xclbin.info *.xclbin.link_summary* *.compile_summary
	-$(RMDIR) .Xil

clean-package:
	-${RMDIR} package
	-${RMDIR} package.build

clean: clean-host clean-accelerators clean-package
	-$(RM) *.log *.package_summary
	-${RMDIR} _x

#
# binary container: kernel.xclbin
#

$(XO): krnl_vadd.cpp
	-@$(RM) $@
	$(VPP) $(VPP_OPTS) -k krnl_vadd --compile -I"$(<D)" --config design.cfg -o"$@" "$<"
$(XCLBIN): $(XO)
	$(VPP) $(VPP_OPTS) --link --config design.cfg -o"$@" $(+)