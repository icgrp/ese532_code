#define CL_HPP_CL_1_2_DEFAULT_BUILD
#define CL_HPP_TARGET_OPENCL_VERSION 120
#define CL_HPP_MINIMUM_OPENCL_VERSION 120
#define CL_HPP_ENABLE_PROGRAM_CONSTRUCTION_FROM_ARRAY_COMPATIBILITY 1
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS

#include "EventTimer.h"
#include <CL/cl2.hpp>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <vector>

#include "MMult.h"
#include "Utilities.h"

static void init_arrays(float *A, float *B, unsigned int num_tests) {
  for (int c = 0; c < num_tests; c++) {
    for (int i = 0; i < N; i++) {
      for (int j = 0; j < N; j++) {
        A[c * N * N + i * N + j] = 1 + i * N + j;
        B[c * N * N + i * N + j] = rand() % (N * N);
      }
    }
  }
}

/**
 * Design principles to achieve performance
 *
 * 1. sds_alloc to guarantee physically contiguous buffer allocation
 *    that enables the most efficient DMA configuration (axidma_simple)
 */
int main(int argc, char *argv[]) {
  EventTimer timer;
  timer.add("Main function");
  unsigned int num_tests = 8192;

  if (argc == 3) {
    num_tests = atoi(argv[2]);
  }

  //   int pipeline_depth = PIPELINE_DEPTH_DEFAULT;

  //   // use pipeline depth from command argument, if passed in
  //   if (argc == 3) {
  //     pipeline_depth = atoi(argv[2]);
  //     if (pipeline_depth > PIPELINE_DEPTH_MAX ||
  //         pipeline_depth < PIPELINE_DEPTH_MIN) {
  //       std::cout << "Please use pipeline depth from " << PIPELINE_DEPTH_MIN
  //                 << " to " << PIPELINE_DEPTH_MAX << std::endl;
  //       return 1;
  //     }
  //   }
  std::cout << "Running " << num_tests << " " << N << "x" << N
            << " floating point mmult on fpga..." << std::endl;

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
  char *fileBuf = read_binary_file(binaryFile, fileBufSize);
  cl::Program::Binaries bins{{fileBuf, fileBufSize}};
  cl::Program program(context, devices, bins, NULL, &err);
  cl::CommandQueue q(context, device, CL_QUEUE_PROFILING_ENABLE, &err);
  cl::Kernel krnl_mmult(program, "mmult_fpga", &err);

  // ------------------------------------------------------------------------------------
  // Step 2: Create buffers and initialize test values
  // ------------------------------------------------------------------------------------
  timer.add("Allocate contiguous OpenCL buffers");
  // Create the buffers and allocate memory
  cl::Buffer in1_buf(context, CL_MEM_ALLOC_HOST_PTR | CL_MEM_READ_ONLY,
                     num_tests * N * N * sizeof(float), NULL, &err);
  cl::Buffer in2_buf(context, CL_MEM_ALLOC_HOST_PTR | CL_MEM_READ_ONLY,
                     num_tests * N * N * sizeof(float), NULL, &err);
  cl::Buffer out_buf_hw(context, CL_MEM_ALLOC_HOST_PTR | CL_MEM_WRITE_ONLY,
                        num_tests * N * N * sizeof(float), NULL, &err);

  timer.add("Set kernel arguments");
  // Map buffers to kernel arguments, thereby assigning them to specific device
  // memory banks
  krnl_mmult.setArg(0, in1_buf);
  krnl_mmult.setArg(1, in2_buf);
  krnl_mmult.setArg(2, out_buf_hw);
  krnl_mmult.setArg(3, num_tests);

  timer.add("Map buffers to userspace pointers");
  // Map host-side buffer memory to user-space pointers
  float *in1 = (float *)q.enqueueMapBuffer(in1_buf, CL_TRUE, CL_MAP_WRITE, 0,
                                           num_tests * N * N * sizeof(float));
  float *in2 = (float *)q.enqueueMapBuffer(in2_buf, CL_TRUE, CL_MAP_WRITE, 0,
                                           num_tests * N * N * sizeof(float));

  timer.add("Populating buffer inputs");
  // Initialize the vectors used in the test
  init_arrays(in1, in2, num_tests);

  // ------------------------------------------------------------------------------------
  // Step 3: Run the kernel
  // ------------------------------------------------------------------------------------

  // Schedule transfer of inputs to device memory, execution of kernel, and
  // transfer of outputs back to host memory
  timer.add("Memory object migration enqueue host->device");
  cl::Event event_sp;
  q.enqueueMigrateMemObjects({in1_buf, in2_buf}, 0 /* 0 means from host*/, NULL,
                             &event_sp);
  clWaitForEvents(1, (const cl_event *)&event_sp);

  timer.add("Launch mmult kernel");
  q.enqueueTask(krnl_mmult, NULL, &event_sp);
  timer.add("Wait for mmult kernel to finish running");
  clWaitForEvents(1, (const cl_event *)&event_sp);

  timer.add("Read back computation results (implicit device->host migration)");
  float *out_hw = (float *)q.enqueueMapBuffer(
      out_buf_hw, CL_TRUE, CL_MAP_READ, 0, num_tests * N * N * sizeof(float));
  
  store_data("output_fpga.bin", out_hw, num_tests * N * N * sizeof(float));
  // ------------------------------------------------------------------------------------
  // Step 4: Release Allocated Resources
  // ------------------------------------------------------------------------------------
  delete[] fileBuf;
  q.enqueueUnmapMemObject(in1_buf, in1);
  q.enqueueUnmapMemObject(in2_buf, in2);
  q.enqueueUnmapMemObject(out_buf_hw, out_hw);
  q.finish();
  timer.finish();
  std::cout << "--------------- Key execution times ---------------"
            << std::endl;
  timer.print();
  return 0;
}
