HOST_CXX ?= aarch64-linux-gnu-g++
VPP ?= ${XILINX_VITIS}/bin/v++
RM = rm -f
RMDIR = rm -rf

VITIS_PLATFORM = u96v2_sbc_base
VITIS_PLATFORM_DIR = ${PLATFORM_REPO_PATHS}
VITIS_PLATFORM_PATH = $(VITIS_PLATFORM_DIR)/u96v2_sbc_base.xpfm

# host compiler global settings
CXXFLAGS += -march=armv8-a+simd -mtune=cortex-a53 -std=c++11 -DVITIS_PLATFORM=$(VITIS_PLATFORM) -D__USE_XOPEN2K8 -I$(XILINX_VIVADO)/include/ -I$(VITIS_PLATFORM_DIR)/sw/$(VITIS_PLATFORM)/linux_domain/sysroot/cortexa72-cortexa53-xilinx-linux/usr/include/xrt/ -O3 -g -Wall -c -fmessage-length=0 --sysroot=$(VITIS_PLATFORM_DIR)/sw/$(VITIS_PLATFORM)/linux_domain/sysroot/cortexa72-cortexa53-xilinx-linux
LDFLAGS += -lxilinxopencl -lpthread -lrt -ldl -lcrypt -lstdc++ -L$(VITIS_PLATFORM_DIR)/sw/$(VITIS_PLATFORM)/linux_domain/sysroot/cortexa72-cortexa53-xilinx-linux/usr/lib/ --sysroot=$(VITIS_PLATFORM_DIR)/sw/$(VITIS_PLATFORM)/linux_domain/sysroot/cortexa72-cortexa53-xilinx-linux

# hardware compiler shared settings
VPP_OPTS = --target hw


#
# OpenCL kernel files
#
XO := mmult.xo
XCLBIN := mmult.xclbin

#
# host files
#
HOST_SOURCES = ./Host.cpp ./common/EventTimer.cpp ./common/Utilities.cpp
HOST_OBJECTS =$(HOST_SOURCES:.cpp=.o)
HOST_EXE = host


all: package/sd_card.img

.cpp.o:
	$(HOST_CXX) $(CXXFLAGS) -I./hls/ -I./common -o "$@" "$<"

$(HOST_EXE): $(HOST_OBJECTS)
	$(HOST_CXX) -o "$@" $(+) $(LDFLAGS)

# $(XO): ./hls/MatrixMultiplication.cpp
# 	$(VPP) $(VPP_OPTS) -k mmult --compile -I"$(<D)" --config ./u96_v2.cfg -o"$@" "$<"

$(XCLBIN): $(XO)
	$(VPP) $(VPP_OPTS) --link --config ./u96_v2.cfg -o"$@" $(+)

package/sd_card.img: $(HOST_EXE) $(XCLBIN) xrt.ini
	$(VPP) --package $(VPP_OPTS) --config ./u96_v2.cfg $(XCLBIN) \
	       --package.out_dir package \
	       --package.sd_file $(HOST_EXE)\
	       --package.kernel_image $(PLATFORM_REPO_PATHS)/sw/u96v2_sbc_base/linux_domain/image/Image \
	       --package.dtb $(PLATFORM_REPO_PATHS)/sw/u96v2_sbc_base/linux_domain/image/system.dtb \
	       --package.rootfs ${PLATFORM_REPO_PATHS}/sw/u96v2_sbc_base/linux_domain/rootfs/rootfs.ext4 \
	       --package.sd_file $(XCLBIN) \
	       --package.sd_file xrt.ini

sd_card_base:
	$(VPP) --package $(VPP_OPTS) --config ./u96_v2.cfg $(XCLBIN) \
	       --package.out_dir package \
	       --package.kernel_image $(PLATFORM_REPO_PATHS)/sw/u96v2_sbc_base/linux_domain/image/Image \
	       --package.dtb $(PLATFORM_REPO_PATHS)/sw/u96v2_sbc_base/linux_domain/image/system.dtb \
	       --package.rootfs ${PLATFORM_REPO_PATHS}/sw/u96v2_sbc_base/linux_domain/rootfs/rootfs.ext4

clean:
	-$(RM) $(HOST_EXE) *.o *.xclbin *.xclbin.sh *.xclbin.info *.xclbin.link_summary* *.compile_summary *.str
	-$(RM) *json *csv *log *summary *.json *.jou
	-$(RMDIR) _x
	-$(RMDIR) .Xil
	-$(RMDIR) .ipcache
	-$(RMDIR) package
