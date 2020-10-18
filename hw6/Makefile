
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
TARGET := hw
PLATFORM := xilinx_aws-vu9p-f1_shell-v04261818_201920_2
HOST_EXE := host
XO := mmult.xo
XCLBIN := mmult.xclbin
S3_BUCKET_NAME := ese532-$(shell aws sts get-caller-identity | grep "Account" | tr -dc '0-9')
S3_DCP_DIR := vitis-dcps
S3_LOG_DIR := vitis-logs
EMAIL :=
SHELL := /bin/bash

# Host building global settings
CXXFLAGS := -I./common -I$(XILINX_XRT)/include/ -I$(XILINX_VIVADO)/include/ -Wall -O3 -g -std=c++11 -L$(XILINX_XRT)/lib/ -lpthread -lrt -lstdc++
CXXFLAGS2 := -lOpenCL

# Kernel compiler & linker global settings
KRNL_COMPILE_OPTS := -t $(TARGET) --config design.cfg --jobs 8 --optimize 3
KRNL_LINK_OPTS := -t $(TARGET) --config design.cfg --jobs 8 --optimize 3

.PHONY: afi
afi: afigen
	source $(AWS_FPGA_REPO_DIR)/hdk_setup.sh
	pip install --user --upgrade boto3
	wait_for_afi.py --afi $(shell cat *afi_id.txt | sed -n '2p' | tr -d '",' | sed 's/.*://') --notify --email $(EMAIL) &

.PHONY: afigen
afigen: all
	aws s3 mb s3://$(S3_BUCKET_NAME) --region us-east-1
	aws s3 mb s3://$(S3_BUCKET_NAME)/$(S3_DCP_DIR)
	touch FILES_GO_HERE.txt
	aws s3 cp FILES_GO_HERE.txt s3://$(S3_BUCKET_NAME)/$(S3_DCP_DIR)
	aws s3 mb s3://$(S3_BUCKET_NAME)/$(S3_LOG_DIR)
	touch LOGS_FILES_GO_HERE.txt
	aws s3 cp LOGS_FILES_GO_HERE.txt s3://$(S3_BUCKET_NAME)/$(S3_LOG_DIR)
	rm -rf to_aws
	$(VITIS_DIR)/tools/create_vitis_afi.sh -xclbin=$(XCLBIN) -s3_bucket=$(S3_BUCKET_NAME) -s3_dcp_key=$(S3_DCP_DIR) -s3_logs_key=$(S3_LOG_DIR)

all:  $(XO) $(XCLBIN) $(HOST_EXE) emconfig

exe: $(HOST_EXE)

xclbin: $(XO) $(XCLBIN)

# Building kernel
$(XO): hls/MatrixMultiplication.cpp
	v++ $(KRNL_COMPILE_OPTS) -c -k mmult -I./common -I./hls/ -o'$@' '$<'
$(XCLBIN): $(XO)
	v++ $(KRNL_LINK_OPTS) -l -o'$@' $(+)

# Building Host
$(HOST_EXE): Host.cpp common/Utilities.cpp common/EventTimer.cpp
	g++ $(CXXFLAGS) $^ -o $@ $(CXXFLAGS2)

.PHONY: emconfig
emconfig:
	emconfigutil --platform $(PLATFORM)

.PHONY: clean
clean:
	$(RM) -rf *.csv *.jou *.run_summary *.dcp to_aws _x *.tar perf* *.bin *.txt *.dSYM *.out *.o *~ rm -f $(HOST_EXE) *mmult.* *.log *.json *.xo
