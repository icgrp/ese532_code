/**********
Copyright (c) 2018, Xilinx, Inc.
All rights reserved.
Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.
3. Neither the name of the copyright holder nor the names of its contributors
may be used to endorse or promote products derived from this software
without specific prior written permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**********/

#define CL_HPP_CL_1_2_DEFAULT_BUILD
#define CL_HPP_TARGET_OPENCL_VERSION 120
#define CL_HPP_MINIMUM_OPENCL_VERSION 120
#define CL_HPP_ENABLE_PROGRAM_CONSTRUCTION_FROM_ARRAY_COMPATIBILITY 1
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS

#include <vector>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <CL/cl2.hpp>
#include "Utilities.h"
#define MATRIX_SIZE (MATRIX_WIDTH*MATRIX_WIDTH)

// Forward declaration of utility functions included at the end of this file
std::vector<cl::Device> get_xilinx_devices();
char* read_binary_file(const std::string &xclbin_file_name, unsigned &nb);
void Multiply_SW(const matrix_type Input_1[MATRIX_WIDTH * MATRIX_WIDTH],
                 const matrix_type Input_2[MATRIX_WIDTH * MATRIX_WIDTH],
		         matrix_type Output[MATRIX_WIDTH * MATRIX_WIDTH]);

// ------------------------------------------------------------------------------------
// Main program
// ------------------------------------------------------------------------------------
int main(int argc, char** argv)
{
// Initialize an event timer we'll use for monitoring the application
    EventTimer timer;
// ------------------------------------------------------------------------------------
// Step 1: Initialize the OpenCL environment 
// ------------------------------------------------------------------------------------ 
    timer.add("OpenCL Initialization");
    cl_int err;
    std::string binaryFile = argv[1];
    unsigned fileBufSize;    
    std::vector<cl::Device> devices = get_xilinx_devices();
    devices.resize(1);
    cl::Device device = devices[0];
    cl::Context context(device, NULL, NULL, NULL, &err);
    char* fileBuf = read_binary_file(binaryFile, fileBufSize);
    cl::Program::Binaries bins{{fileBuf, fileBufSize}};
    cl::Program program(context, devices, bins, NULL, &err);
    cl::CommandQueue q(context, device, CL_QUEUE_PROFILING_ENABLE, &err);
    cl::Kernel krnl_mmult(program,"Multiply_HW", &err);

// ------------------------------------------------------------------------------------
// Step 2: Create buffers and initialize test values
// ------------------------------------------------------------------------------------
    timer.add("Allocate contiguous OpenCL buffers");
    // Create the buffers and allocate memory   
    cl::Buffer in1_buf(context, CL_MEM_ALLOC_HOST_PTR | CL_MEM_READ_ONLY,  sizeof(matrix_type) * MATRIX_SIZE, NULL, &err);
    cl::Buffer in2_buf(context, CL_MEM_ALLOC_HOST_PTR | CL_MEM_READ_ONLY,  sizeof(matrix_type) * MATRIX_SIZE, NULL, &err);
    cl::Buffer out_buf_hw(context, CL_MEM_ALLOC_HOST_PTR | CL_MEM_WRITE_ONLY, sizeof(matrix_type) * MATRIX_SIZE, NULL, &err);
    cl::Buffer out_buf_sw(context, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR), sizeof(matrix_type) * MATRIX_SIZE, NULL, &err);

    timer.add("Set kernel arguments");  
    // Map buffers to kernel arguments, thereby assigning them to specific device memory banks
    krnl_mmult.setArg(0, in1_buf);
    krnl_mmult.setArg(1, in2_buf);
    krnl_mmult.setArg(2, out_buf_hw);

    timer.add("Map buffers to userspace pointers");
    // Map host-side buffer memory to user-space pointers
    matrix_type *in1 = (matrix_type *)q.enqueueMapBuffer(in1_buf, CL_TRUE, CL_MAP_WRITE, 0, sizeof(matrix_type) * MATRIX_SIZE);
    matrix_type *in2 = (matrix_type *)q.enqueueMapBuffer(in2_buf, CL_TRUE, CL_MAP_WRITE, 0, sizeof(matrix_type) * MATRIX_SIZE); 
    matrix_type *out_sw = (matrix_type *)q.enqueueMapBuffer(out_buf_sw, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, sizeof(matrix_type) * MATRIX_SIZE);

    timer.add("Populating buffer inputs");
    // Initialize the vectors used in the test
    Randomize_matrix(in1);
    Randomize_matrix(in2);

// ------------------------------------------------------------------------------------
// Step 3: Run the kernel
// ------------------------------------------------------------------------------------
    timer.add("Set kernel arguments");
    // Set kernel arguments
    krnl_mmult.setArg(0, in1_buf);
    krnl_mmult.setArg(1, in2_buf);
    krnl_mmult.setArg(2, out_buf_hw);

    // Schedule transfer of inputs to device memory, execution of kernel, and transfer of outputs back to host memory
    timer.add("Memory object migration enqueue host->device");
    cl::Event event_sp;
    q.enqueueMigrateMemObjects({in1_buf, in2_buf}, 0 /* 0 means from host*/, NULL, &event_sp); 
    clWaitForEvents(1, (const cl_event *)&event_sp);

    timer.add("OCL Enqueue task");
    q.enqueueTask(krnl_mmult, NULL, &event_sp);
    timer.add("Wait for Multiply_HW kernel to complete");
    clWaitForEvents(1, (const cl_event *)&event_sp);
    
    timer.add("Read back computation results (implicit device->host migration)");
    matrix_type *out_hw = (matrix_type *)q.enqueueMapBuffer(out_buf_hw, CL_TRUE, CL_MAP_READ, 0, sizeof(matrix_type) * MATRIX_SIZE);
    
// ------------------------------------------------------------------------------------
// Step 4: Check Results and Release Allocated Resources
// ------------------------------------------------------------------------------------
    timer.add("Multiply_SW run");
    Multiply_SW(in1, in2, out_sw);
    timer.finish();
    bool match = Compare_matrices(out_sw, out_hw);

    delete[] fileBuf;
    q.enqueueUnmapMemObject(in1_buf, in1);
    q.enqueueUnmapMemObject(in2_buf, in2);
    q.enqueueUnmapMemObject(out_buf_sw, out_sw);
    q.enqueueUnmapMemObject(out_buf_hw, out_hw);
    q.finish();

    std::cout << "--------------- Key execution times ---------------" << std::endl;
    timer.print();

    std::cout << "TEST " << (match ? "PASSED" : "FAILED") << std::endl; 
    return (match ? EXIT_SUCCESS : EXIT_FAILURE);
}



// ------------------------------------------------------------------------------------
// Utility functions
// ------------------------------------------------------------------------------------
std::vector<cl::Device> get_xilinx_devices() 
{
    size_t i;
    cl_int err;
    std::vector<cl::Platform> platforms;
    err = cl::Platform::get(&platforms);
    cl::Platform platform;
    for (i  = 0 ; i < platforms.size(); i++){
        platform = platforms[i];
        std::string platformName = platform.getInfo<CL_PLATFORM_NAME>(&err);
        if (platformName == "Xilinx"){
            std::cout << "INFO: Found Xilinx Platform" << std::endl;
            break;
        }
    }
    if (i == platforms.size()) {
        std::cout << "ERROR: Failed to find Xilinx platform" << std::endl;
        exit(EXIT_FAILURE);
    }
   
    //Getting ACCELERATOR Devices and selecting 1st such device 
    std::vector<cl::Device> devices;
    err = platform.getDevices(CL_DEVICE_TYPE_ACCELERATOR, &devices);
    return devices;
}
   
char* read_binary_file(const std::string &xclbin_file_name, unsigned &nb) 
{
    if(access(xclbin_file_name.c_str(), R_OK) != 0) {
        printf("ERROR: %s xclbin not available please build\n", xclbin_file_name.c_str());
        exit(EXIT_FAILURE);
    }
    //Loading XCL Bin into char buffer 
    std::cout << "INFO: Loading '" << xclbin_file_name << "'\n";
    std::ifstream bin_file(xclbin_file_name.c_str(), std::ifstream::binary);
    bin_file.seekg (0, bin_file.end);
    nb = bin_file.tellg();
    bin_file.seekg (0, bin_file.beg);
    char *buf = new char [nb];
    bin_file.read(buf, nb);
    return buf;
}

void Multiply_SW(const matrix_type Input_1[MATRIX_WIDTH * MATRIX_WIDTH],
                 const matrix_type Input_2[MATRIX_WIDTH * MATRIX_WIDTH],
		         matrix_type Output[MATRIX_WIDTH * MATRIX_WIDTH])
{
  for (int i = 0; i < MATRIX_WIDTH; i++)
    for (int j = 0; j < MATRIX_WIDTH; j++)
    {
      matrix_type Result = 0;
      for (int k = 0; k < MATRIX_WIDTH; k++)
        Result += Input_1[i * MATRIX_WIDTH + k] * Input_2[k * MATRIX_WIDTH + j];
      Output[i * MATRIX_WIDTH + j] = Result;
    }
}
