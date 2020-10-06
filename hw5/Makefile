
#######################################################################################
.PHONY: help
help:
	@echo "Makefile Usage:"
	@echo "  make baseline"
	@echo "      Command to build the baseline."
	@echo ""
	@echo "  make fpga TARGET=<sw_emu/hw_emu/hw> PLATFORM=<FPGA platform>"
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
#######################################################################################
TARGET := sw_emu
PLATFORM := xilinx_aws-vu9p-f1_shell-v04261818_201920_2
HOST_EXE := host
XO := mmult.$(TARGET).$(PLATFORM).xo
XCLBIN := mmult.$(TARGET).$(PLATFORM).xclbin

# Host building global settings
CXXFLAGS := -I./common -I$(XILINX_XRT)/include/ -I$(XILINX_VIVADO)/include/ -Wall -O3 -g -std=c++11 -L$(XILINX_XRT)/lib/ -lpthread -lrt -lstdc++
CXXFLAGS2 := -lOpenCL

# Kernel compiler & linker global settings
KRNL_COMPILE_OPTS := -t $(TARGET) --config design.cfg
KRNL_LINK_OPTS := -t $(TARGET) --config design.cfg

baseline: cpu/MatrixMultiplication.cpp common/Utilities.cpp
	g++ $(CXXFLAGS) -o '$@' '$<' $(CXXFLAGS2)

fpga:  $(XO) $(XCLBIN) $(HOST_EXE) emconfig

exe: $(HOST_EXE)

xclbin: $(XO) $(XCLBIN)

# Building kernel
$(XO): fpga/hls/MatrixMultiplication.cpp
	v++ $(KRNL_COMPILE_OPTS) -c -k Multiply_HW -I./common -o'$@' '$<'
$(XCLBIN): $(XO)
	v++ $(KRNL_LINK_OPTS) -l -o'$@' $(+)

# Building Host
$(HOST_EXE): fpga/Host.cpp common/Utilities.cpp
	g++ $(CXXFLAGS) -o '$@' '$<' $(CXXFLAGS2)

.PHONY: emconfig
emconfig:
	emconfigutil --platform $(PLATFORM)

.PHONY: clean
clean:
	$(RM) -rf perf* *.bin *.txt *.dSYM *.out *.o *~ rm -f $(HOST_EXE) *mmult.$(TARGET).$(PLATFORM).* *.log *.json *.xo baseline
