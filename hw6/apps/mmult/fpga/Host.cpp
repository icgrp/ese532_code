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

static void init_arrays(float *A[NUM_MAT],  
                        float *B[NUM_MAT])
{
     for (int m = 0; m < NUM_MAT; m++) {
    	for (int c = 0; c < CHUNKS; c++) {
          for (int i = 0; i < N; i++) {
               for (int j = 0; j < N; j++) {
                    A[m][ c * N * N + i * N + j] = 1+i*N+j;
                    B[m][ c * N * N + i * N + j] = rand() % (N * N);
               }
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
  std::cout << "Running " << CHUNKS << "x" <<NUM_TESTS << " iterations of " << N << "x" << N
               << " task pipelined floating point mmult..." << std::endl;
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
  cl::Buffer A_buf[NUM_MAT];
  cl::Buffer B_buf[NUM_MAT];
  cl::Buffer C_buf[NUM_MAT];
  float *A[NUM_MAT], *B[NUM_MAT], *C[NUM_MAT];

  for (int m = 0; m < NUM_MAT; m++) {
    A_buf[m] = cl::Buffer(context, CL_MEM_ALLOC_HOST_PTR | CL_MEM_READ_ONLY,
                     CHUNKS * N * N * sizeof(float), NULL, &err);
    B_buf[m] = cl::Buffer(context, CL_MEM_ALLOC_HOST_PTR | CL_MEM_READ_ONLY,
                     CHUNKS * N * N * sizeof(float), NULL, &err);
    C_buf[m] = cl::Buffer(context, CL_MEM_ALLOC_HOST_PTR | CL_MEM_WRITE_ONLY,
                        CHUNKS * N * N * sizeof(float), NULL, &err);
    
    // Map host-side buffer memory to user-space pointers
    A[m] = (float *)q.enqueueMapBuffer(A_buf[m], CL_TRUE, CL_MAP_WRITE, 0,
                                            CHUNKS * N * N * sizeof(float));
    B[m] = (float *)q.enqueueMapBuffer(B_buf[m], CL_TRUE, CL_MAP_WRITE, 0,
                                            CHUNKS * N * N * sizeof(float));
  }
  
  timer.add("Populating buffer inputs");
  // Initialize the vectors used in the test
  init_arrays(A, B);
  
  // ------------------------------------------------------------------------------------
  // Step 3: Run the kernel
  // ------------------------------------------------------------------------------------

  timer.add("Running kernel");
  for (int i = 0; i < NUM_TESTS; i++) {
    // Map buffers to kernel arguments, thereby assigning them to specific device
    // memory banks
    krnl_mmult.setArg(0, A_buf[i%NUM_MAT]);
    krnl_mmult.setArg(1, B_buf[i%NUM_MAT]);
    krnl_mmult.setArg(2, C_buf[i%NUM_MAT]);
    // Schedule transfer of inputs to device memory, execution of kernel, and
  // transfer of outputs back to host memory

  cl::Event event_sp;
  q.enqueueMigrateMemObjects({A_buf[i%NUM_MAT], B_buf[i%NUM_MAT]}, 0 /* 0 means from host*/, NULL,
                             &event_sp);
  //clWaitForEvents(1, (const cl_event *)&event_sp);
  q.enqueueTask(krnl_mmult, NULL, &event_sp);
  //clWaitForEvents(1, (const cl_event *)&event_sp);
  C[i%NUM_MAT] = (float *)q.enqueueMapBuffer(
      C_buf[i%NUM_MAT], CL_TRUE, CL_MAP_READ, 0, CHUNKS * N * N * sizeof(float));
  }

  timer.add("Read back computation results (implicit device->host migration)");
  for (int m = 0; m < NUM_MAT; m++) {
    q.enqueueUnmapMemObject(A_buf[m], A[m]);
    q.enqueueUnmapMemObject(B_buf[m], B[m]);
    q.enqueueUnmapMemObject(C_buf[m], C[m]);
  }
  
  timer.add("Writing output to output_fpga.bin");
  FILE *file = fopen("output_fpga.bin", "wb");
  for (int m = 0; m < NUM_MAT; m++) {
    fwrite(C[m], sizeof(float), CHUNKS * N * N, file);
  }
  fclose(file);

  
  // ------------------------------------------------------------------------------------
  // Step 4: Release Allocated Resources
  // ------------------------------------------------------------------------------------
  delete[] fileBuf;
  
  q.finish();
  timer.finish();
  std::cout << "--------------- Key execution times ---------------"
            << std::endl;
  timer.print();
  return 0;
}
