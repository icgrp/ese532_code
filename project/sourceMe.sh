#!/bin/bash
XLXDIR='/mnt/pollux/software/xilinx/2020.2'
# change the PLATFORM_REPO_PATHS
export PLATFORM_REPO_PATHS=/home1/e/ese5320/u96v2_sbc_base
export XILINXD_LICENSE_FILE=/mnt/pollux/software/xilinx/.Xilinx/
#export LIBRARY_PATH=/usr/x86_64-suse-linux

export XILINX_VITIS=${XLXDIR}/Vitis/2020.2
if [ -n "${PATH}" ]; then
  export PATH=${XLXDIR}/Vitis/2020.2/bin:${XLXDIR}/Vitis/2020.2/gnu/microblaze/lin/bin:${XLXDIR}/Vitis/2020.2/gnu/arm/lin/bin:${XLXDIR}/Vitis/2020.2/gnu/microblaze/linux_toolchain/lin64_le/bin:${XLXDIR}/Vitis/2020.2/gnu/aarch32/lin/gcc-arm-linux-gnueabi/bin:${XLXDIR}/Vitis/2020.2/gnu/aarch32/lin/gcc-arm-none-eabi/bin:${XLXDIR}/Vitis/2020.2/gnu/aarch64/lin/aarch64-linux/bin:${XLXDIR}/Vitis/2020.2/gnu/aarch64/lin/aarch64-none/bin:${XLXDIR}/Vitis/2020.2/gnu/armr5/lin/gcc-arm-none-eabi/bin:${XLXDIR}/Vitis/2020.2/tps/lnx64/cmake-3.3.2/bin:${XLXDIR}/Vitis/2020.2/cardano/bin:$PATH
else
  export PATH=${XLXDIR}/Vitis/2020.2/bin:${XLXDIR}/Vitis/2020.2/gnu/microblaze/lin/bin:${XLXDIR}/Vitis/2020.2/gnu/arm/lin/bin:${XLXDIR}/Vitis/2020.2/gnu/microblaze/linux_toolchain/lin64_le/bin:${XLXDIR}/Vitis/2020.2/gnu/aarch32/lin/gcc-arm-linux-gnueabi/bin:${XLXDIR}/Vitis/2020.2/gnu/aarch32/lin/gcc-arm-none-eabi/bin:${XLXDIR}/Vitis/2020.2/gnu/aarch64/lin/aarch64-linux/bin:${XLXDIR}/Vitis/2020.2/gnu/aarch64/lin/aarch64-none/bin:${XLXDIR}/Vitis/2020.2/gnu/armr5/lin/gcc-arm-none-eabi/bin:${XLXDIR}/Vitis/2020.2/tps/lnx64/cmake-3.3.2/bin:${XLXDIR}/Vitis/2020.2/cardano/bin
fi

export XILINX_HLS=${XLXDIR}/Vitis_HLS/2020.2
if [ -n "${PATH}" ]; then
  export PATH=${XLXDIR}/Vitis_HLS/2020.2/bin:$PATH
else
  export PATH=${XLXDIR}/Vitis_HLS/2020.2/bin
fi

export XILINX_VIVADO=${XLXDIR}/Vivado/2020.2
if [ -n "${PATH}" ]; then
  export PATH=${XLXDIR}/Vivado/2020.2/bin:$PATH
else
  export PATH=${XLXDIR}/Vivado/2020.2/bin
fi


if [ -n "${PATH}" ]; then
  export PATH=${XILXDIR}/Model_Composer/2020.2/bin:$PATH
else
  export PATH=${XILXDIR}/Model_Composer/2020.2/bin
fi

export PATH=/usr/sbin:$PATH

# run with four cpus since there are other people using biglab
# and your processes can get killed if they hog all the cpus.
#make fpga -j4

