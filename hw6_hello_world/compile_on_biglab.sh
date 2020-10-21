#!/bin/bash

XLXDIR='/mnt/pollux/software/xilinx/2020.1'
export PLATFORM_REPO_PATHS=/home1/s/stahmed/ese532_platforms
export XILINXD_LICENSE_FILE=/mnt/pollux/software/xilinx/.Xilinx/
export XILINX_VITIS=${XLXDIR}/Vitis/2020.1
if [ -n "${PATH}" ]; then
  export PATH=${XLXDIR}/Vitis/2020.1/bin:${XLXDIR}/Vitis/2020.1/gnu/microblaze/lin/bin:${XLXDIR}/Vitis/2020.1/gnu/arm/lin/bin:${XLXDIR}/Vitis/2020.1/gnu/microblaze/linux_toolchain/lin64_le/bin:${XLXDIR}/Vitis/2020.1/gnu/aarch32/lin/gcc-arm-linux-gnueabi/bin:${XLXDIR}/Vitis/2020.1/gnu/aarch32/lin/gcc-arm-none-eabi/bin:${XLXDIR}/Vitis/2020.1/gnu/aarch64/lin/aarch64-linux/bin:${XLXDIR}/Vitis/2020.1/gnu/aarch64/lin/aarch64-none/bin:${XLXDIR}/Vitis/2020.1/gnu/armr5/lin/gcc-arm-none-eabi/bin:${XLXDIR}/Vitis/2020.1/tps/lnx64/cmake-3.3.2/bin:${XLXDIR}/Vitis/2020.1/cardano/bin:$PATH
else
  export PATH=${XLXDIR}/Vitis/2020.1/bin:${XLXDIR}/Vitis/2020.1/gnu/microblaze/lin/bin:${XLXDIR}/Vitis/2020.1/gnu/arm/lin/bin:${XLXDIR}/Vitis/2020.1/gnu/microblaze/linux_toolchain/lin64_le/bin:${XLXDIR}/Vitis/2020.1/gnu/aarch32/lin/gcc-arm-linux-gnueabi/bin:${XLXDIR}/Vitis/2020.1/gnu/aarch32/lin/gcc-arm-none-eabi/bin:${XLXDIR}/Vitis/2020.1/gnu/aarch64/lin/aarch64-linux/bin:${XLXDIR}/Vitis/2020.1/gnu/aarch64/lin/aarch64-none/bin:${XLXDIR}/Vitis/2020.1/gnu/armr5/lin/gcc-arm-none-eabi/bin:${XLXDIR}/Vitis/2020.1/tps/lnx64/cmake-3.3.2/bin:${XLXDIR}/Vitis/2020.1/cardano/bin
fi
make fpga