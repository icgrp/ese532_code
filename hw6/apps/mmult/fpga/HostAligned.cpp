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

int main(int argc, char *argv[]) {
  EventTimer timer1, timer2;
  timer1.add("Main program");

  std::cout << "Running " << CHUNKS << "x" <<NUM_TESTS << " iterations of " << N << "x" << N
               << " task pipelined floating point mmult..." << std::endl;
  // ------------------------------------------------------------------------------------
  // Step 1: Initialize the OpenCL environment
  // ------------------------------------------------------------------------------------
  timer2.add("OpenCL Initialization");
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
  timer2.add("Allocate contiguous OpenCL buffers");
  // Create the buffers and allocate memory
  cl::Buffer A_buf[NUM_MAT];
  cl::Buffer B_buf[NUM_MAT];
  cl::Buffer C_buf[NUM_MAT];
  float *A[NUM_MAT], *B[NUM_MAT], *C[NUM_MAT];
  size_t elements_per_iteration = CHUNKS * N * N;
  size_t bytes_per_iteration = elements_per_iteration * sizeof(float);

  for (int m = 0; m < NUM_MAT; m++) {
    // @TODO:
    // Remove CL_MEM_USE_HOST_PTR and 
    // Use enqueueMapBuffer to get user side pointer

    A[m] = (float *)malloc(bytes_per_iteration);
    B[m] = (float *)malloc(bytes_per_iteration);
    C[m] = (float *)malloc(bytes_per_iteration);
    if (!A[m] || !B[m] || !C[m]) {
      if (A[m])
        free(A[m]);
      if (B[m])
        free(B[m]);
      if (C[m])
        free(C[m]);
      return 2;
    }

    A_buf[m] = cl::Buffer(context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY,
                     bytes_per_iteration, A[m], &err);
    B_buf[m] = cl::Buffer(context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY,
                     bytes_per_iteration, B[m], &err);
    C_buf[m] = cl::Buffer(context, CL_MEM_USE_HOST_PTR | CL_MEM_WRITE_ONLY,
                        bytes_per_iteration, C[m], &err);
  }
  
  timer2.add("Populating buffer inputs");
  init_arrays(A, B);
  
  // ------------------------------------------------------------------------------------
  // Step 3: Run the kernel
  // ------------------------------------------------------------------------------------

  timer2.add("Running kernel");
  std::vector<cl::Event> write_events;
  for (int i = 0; i < NUM_TESTS; i++) {
    std::vector<cl::Event> exec_events, read_events;
    cl::Event write_ev, exec_ev, read_ev;

    krnl_mmult.setArg(0, A_buf[i%NUM_MAT]);
    krnl_mmult.setArg(1, B_buf[i%NUM_MAT]);
    krnl_mmult.setArg(2, C_buf[i%NUM_MAT]);
    if(i == 0){
        q.enqueueMigrateMemObjects({A_buf[i%NUM_MAT], B_buf[i%NUM_MAT]}, 0 /* 0 means from host*/, NULL,
                              &write_ev);
    } else {

        q.enqueueMigrateMemObjects({A_buf[i%NUM_MAT], B_buf[i%NUM_MAT]}, 0 /* 0 means from host*/, &write_events,
                              &write_ev);
        write_events.pop_back();
    }
    write_events.push_back(write_ev);
    q.enqueueTask(krnl_mmult, &write_events, &exec_ev);
    exec_events.push_back(exec_ev);
    q.enqueueMigrateMemObjects({C_buf[i%NUM_MAT]}, CL_MIGRATE_MEM_OBJECT_HOST, &exec_events, &read_ev);
    read_events.push_back(read_ev);
  }

  q.finish();

  // ------------------------------------------------------------------------------------
  // Step 4: Release Allocated Resources
  // ------------------------------------------------------------------------------------
  timer2.add("Read back computation results (implicit device->host migration)");
  // @TODO:
  // Add another loop here and release cl::Buffer and
  // user side pointers using enqueueUnmapMemObject

  timer2.add("Writing output to output_fpga.bin");
  FILE *file = fopen("output_fpga.bin", "wb");
  for (int m = 0; m < NUM_MAT; m++) {
    fwrite(C[m], 1, bytes_per_iteration, file);
    // @TODO:
    // Remove the free-s from here, since using
    // enqueueUnmapMemObject
    free(A[m]);
    free(B[m]);
    free(C[m]);
  }
  fclose(file);

  delete[] fileBuf;
  
  
  timer2.finish();
  std::cout << "--------------- Key execution times ---------------"
            << std::endl;
  timer2.print();

  timer1.finish();
  std::cout << "--------------- Total time ---------------"
            << std::endl;
  timer1.print();
  return 0;
}