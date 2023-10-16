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

static void init_arrays(float *A[NUM_MAT], float *B[NUM_MAT])
{
    for (int m = 0; m < NUM_MAT; m++)
    {
        for (int c = 0; c < CHUNKS; c++)
        {
            for (int i = 0; i < N; i++)
            {
                for (int j = 0; j < N; j++)
                {
                    A[m][ c * N * N + i * N + j] = 1+i*N+j;
                    B[m][ c * N * N + i * N + j] = rand() % (N * N);
                }
            }
        }
    }
}

int main(int argc, char *argv[])
{
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

    size_t elements_per_iteration = CHUNKS * N * N;
    size_t bytes_per_iteration = elements_per_iteration * sizeof(float);

    cl::Buffer a_buf[NUM_MAT];
    cl::Buffer b_buf[NUM_MAT];
    cl::Buffer c_buf[NUM_MAT];
    for(int i = 0; i < NUM_MAT; i++)
    {
        a_buf[i] = cl::Buffer(context, CL_MEM_READ_ONLY, bytes_per_iteration, NULL, &err);
        b_buf[i] = cl::Buffer(context, CL_MEM_READ_ONLY, bytes_per_iteration, NULL, &err);
        c_buf[i] = cl::Buffer(context, CL_MEM_WRITE_ONLY, bytes_per_iteration, NULL, &err);
    }

    float *a[NUM_MAT];
    float *b[NUM_MAT];
    float *c[NUM_MAT];
    for(int i = 0; i < NUM_MAT; i++)
    {
        a[i] = (float*)q.enqueueMapBuffer(a_buf[i], CL_TRUE, CL_MAP_WRITE, 0, bytes_per_iteration);
        b[i] = (float*)q.enqueueMapBuffer(b_buf[i], CL_TRUE, CL_MAP_WRITE, 0, bytes_per_iteration);
        c[i] = (float*)q.enqueueMapBuffer(c_buf[i], CL_TRUE, CL_MAP_READ, 0, bytes_per_iteration);
    }

    timer2.add("Populating buffer inputs");
    init_arrays(a, b);

    // ------------------------------------------------------------------------------------
    // Step 3: Run the kernel
    // ------------------------------------------------------------------------------------

    timer2.add("Running kernel");
    std::vector<cl::Event> read_events;
    for (int i = 0; i < NUM_TESTS; i++)
    {
        std::vector<cl::Event> exec_events, write_events;
        cl::Event write_ev, exec_ev, read_ev;

        krnl_mmult.setArg(0, a_buf[i%NUM_MAT]);
        krnl_mmult.setArg(1, b_buf[i%NUM_MAT]);
        krnl_mmult.setArg(2, c_buf[i%NUM_MAT]);
        if(i == 0)
        {
            q.enqueueMigrateMemObjects({a_buf[i%NUM_MAT], b_buf[i%NUM_MAT]}, 0 /* 0 means from host*/, NULL, &write_ev);
        }
        else
        {
            q.enqueueMigrateMemObjects({a_buf[i%NUM_MAT], b_buf[i%NUM_MAT]}, 0 /* 0 means from host*/, &read_events, &write_ev);
            read_events.pop_back();
        }

        write_events.push_back(write_ev);
        q.enqueueTask(krnl_mmult, &write_events, &exec_ev);
        exec_events.push_back(exec_ev);
        q.enqueueMigrateMemObjects({c_buf[i%NUM_MAT]}, CL_MIGRATE_MEM_OBJECT_HOST, &exec_events, &read_ev);
        read_events.push_back(read_ev);
    }

    q.finish();

    // ------------------------------------------------------------------------------------
    // Step 4: Release Allocated Resources
    // ------------------------------------------------------------------------------------

    timer2.add("Writing output to output_fpga.bin");
    FILE *file = fopen("output_fpga.bin", "wb");

    for (int i = 0; i < NUM_MAT; i++)
    {
      fwrite(c[i], 1, bytes_per_iteration, file);
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
