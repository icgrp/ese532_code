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
VPP_OPTS = --target hw --platform $(VITIS_PLATFORM) --debug --save-temps

# link config (stream connection between the two kernels + clock assignment)
LINK_CFG = krnl_stream_vadd_vmult.ini

# optional XRT runtime config (profiling/trace); packaged only if present
XRT_INI = $(wildcard xrt.ini)


#
# OpenCL kernel files
#
XO_VADD := krnl_stream_vadd.xo
XO_VMULT := krnl_stream_vmult.xo
XOS := $(XO_VADD) $(XO_VMULT)
XCLBIN := krnl_stream_vadd_vmult.xclbin

#
# host files
#
HOST_SOURCES = ./host.cpp ./xcl2.cpp
HOST_OBJECTS =$(HOST_SOURCES:.cpp=.o)
HOST_EXE = host


all: package/sd_card.img

.cpp.o:
	$(HOST_CXX) $(CXXFLAGS) -I. -o "$@" "$<"

$(HOST_OBJECTS): xcl2.hpp

$(HOST_EXE): $(HOST_OBJECTS)
	$(HOST_CXX) -o "$@" $(+) $(LDFLAGS)

$(XO_VADD): ./krnl_stream_vadd.cpp
	$(VPP) $(VPP_OPTS) -k krnl_stream_vadd --compile -I"$(<D)" -o"$@" "$<"

$(XO_VMULT): ./krnl_stream_vmult.cpp
	$(VPP) $(VPP_OPTS) -k krnl_stream_vmult --compile -I"$(<D)" -o"$@" "$<"

$(XCLBIN): $(XOS) $(LINK_CFG)
	$(VPP) $(VPP_OPTS) --link --config ./$(LINK_CFG) -o"$@" $(XOS)

package/sd_card.img: $(HOST_EXE) $(XCLBIN) $(XRT_INI)
	$(VPP) --package $(VPP_OPTS) $(XCLBIN) \
	       --package.out_dir package \
	       --package.sd_file $(HOST_EXE) \
	       --package.kernel_image $(PLATFORM_REPO_PATHS)/sw/u96v2_sbc_base/linux_domain/image/Image \
	       --package.dtb $(PLATFORM_REPO_PATHS)/sw/u96v2_sbc_base/linux_domain/image/system.dtb \
	       --package.rootfs ${PLATFORM_REPO_PATHS}/sw/u96v2_sbc_base/linux_domain/rootfs/rootfs.ext4 \
	       --package.sd_file $(XCLBIN) \
	       $(addprefix --package.sd_file ,$(XRT_INI))

clean:
	-$(RM) $(HOST_EXE) *.o *.xo *.xclbin *.xclbin.sh *.xclbin.info *.xclbin.link_summary* *.compile_summary *.str
	-$(RM) *json *csv *log *summary *.json *.jou
	-$(RMDIR) _x
	-$(RMDIR) .Xil
	-$(RMDIR) .ipcache
	-$(RMDIR) package

.PHONY: all clean
