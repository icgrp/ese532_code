#ifndef SRC_UTILITIES_
#define SRC_UTILITIES_

#define CL_HPP_CL_1_2_DEFAULT_BUILD
#define CL_HPP_TARGET_OPENCL_VERSION 120
#define CL_HPP_MINIMUM_OPENCL_VERSION 120
#define CL_HPP_ENABLE_PROGRAM_CONSTRUCTION_FROM_ARRAY_COMPATIBILITY 1
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS

#include <vector>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <stdio.h>
#include <CL/cl2.hpp>
#include "EventTimer.h"

std::vector<cl::Device> get_xilinx_devices();
char* read_binary_file(const std::string &xclbin_file_name, unsigned &nb);
void store_data(const char *filename, float *data, unsigned int size);

#endif