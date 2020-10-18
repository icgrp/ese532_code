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
#include <cstdint>
#include <CL/cl2.hpp>
#include "EventTimer.h"

#include "mmult_accel.h"

static void init_arrays(float *A[NUM_MAT],
                        float *B[NUM_MAT],
                        float *C_sw[NUM_MAT],
                        float *C[NUM_MAT])
{
     for (int m = 0; m < NUM_MAT; m++)
     {
          for (int c = 0; c < CHUNKS; c++)
          {
               for (int i = 0; i < N; i++)
               {
                    for (int j = 0; j < N; j++)
                    {
                         A[m][c * N * N + i * N + j] = 1 + i * N + j;
                         B[m][c * N * N + i * N + j] = rand() % (N * N);
                         C_sw[m][c * N * N + i * N + j] = 0.0;
                         C[m][c * N * N + i * N + j] = 0.0;
                    }
               }
          }
     }
}

void mmult_golden(float *A, float *B, float *C)
{
     for (int c = 0; c < CHUNKS; c++)
          for (int row = 0; row < N; row++)
          {
               for (int col = 0; col < N; col++)
               {
                    float result = 0.0;
                    for (int k = 0; k < N; k++)
                    {
                         result += A[c * N * N + row * N + k] * B[c * N * N + k * N + col];
                    }
                    C[c * N * N + row * N + col] = result;
               }
          }
}

static int result_check(float *C[NUM_MAT], float *C_sw[NUM_MAT])
{
     for (int m = 0; m < NUM_MAT; m++)
     {
          for (int i = 0; i < CHUNKS * N * N; i++)
          {
               if (C_sw[m][i] != C[m][i])
               {
                    std::cout << "Mismatch: data index=" << i << " d=" << C_sw[m][i]
                              << ", dout=" << C[m][i] << std::endl;
                    return 1;
               }
          }
     }
     return 0;
}

int mmult_test(float *A[NUM_MAT],
               float *B[NUM_MAT],
               float *C_sw[NUM_MAT],
               float *C[NUM_MAT],
               int pipeline_depth)
{
     std::cout << "Running " << CHUNKS << "x" << NUM_TESTS << " iterations of " << N << "x" << N
               << " task pipelined floating point mmult..." << std::endl;

     perf_counter hw_ctr, sw_ctr;

     init_arrays(A, B, C_sw, C);

     uint64_t start = sds_clock_counter();
     sw_ctr.start();

     for (int i = 0; i < NUM_TESTS; i++)
     {
          mmult_golden(A[i % NUM_MAT], B[i % NUM_MAT], C_sw[i % NUM_MAT]);
     }

     sw_ctr.stop();

     hw_ctr.start();

     for (int i = 0; i < NUM_TESTS; i++)
     {
          mmult_accel(A[i % NUM_MAT], B[i % NUM_MAT], C[i % NUM_MAT]);
     }

     hw_ctr.stop();
     uint64_t stop = sds_clock_counter();
     uint64_t sw_cycles = sw_ctr.avg_cpu_cycles() / NUM_TESTS;
     uint64_t hw_cycles = hw_ctr.avg_cpu_cycles() / NUM_TESTS;
     double speedup = (double)sw_cycles / (double)hw_cycles;

     std::cout << "Average number of CPU cycles running mmult in software: "
               << sw_cycles << std::endl;
     std::cout << "Average number of CPU cycles running mmult in hardware: "
               << hw_cycles << std::endl;
     std::cout << "Speed up: " << speedup << std::endl;
     std::cout << "overall runtime: " << (stop - start) << " cycles" << std::endl;

     int result = 0;
     for (int i = 0; !result && i < NUM_MAT; i++)
          result = result_check(C, C_sw);
     return result;
}

/**
 * Design principles to achieve performance
 *
 * 1. sds_alloc to guarantee physically contiguous buffer allocation
 *    that enables the most efficient DMA configuration (axidma_simple)
 */
int main(int argc, char *argv[])
{
     int pipeline_depth = PIPELINE_DEPTH_DEFAULT;
     float *A[NUM_MAT], *B[NUM_MAT], *C_sw[NUM_MAT], *C[NUM_MAT];

     // use pipeline depth from command argument, if passed in
     if (argc == 2)
     {
          pipeline_depth = atoi(argv[1]);
          if (pipeline_depth > PIPELINE_DEPTH_MAX || pipeline_depth < PIPELINE_DEPTH_MIN)
          {
               std::cout << "Please use pipeline depth from " << PIPELINE_DEPTH_MIN
                         << " to " << PIPELINE_DEPTH_MAX << std::endl;
               return 1;
          }
     }
     for (int m = 0; m < NUM_MAT; m++)
     {
          A[m] = (float *)sds_alloc(CHUNKS * N * N * sizeof(float));
          B[m] = (float *)sds_alloc(CHUNKS * N * N * sizeof(float));
          C[m] = (float *)sds_alloc(CHUNKS * N * N * sizeof(float));
          C_sw[m] = (float *)sds_alloc(CHUNKS * N * N * sizeof(float));

          if (!A[m] || !B[m] || !C[m] || !C_sw[m])
          {
               if (A[m])
                    sds_free(A[m]);
               if (B[m])
                    sds_free(B[m]);
               if (C[m])
                    sds_free(C[m]);
               if (C_sw[m])
                    sds_free(C_sw[m]);
               return 2;
          }
     }
     int test_failed = mmult_test(A, B, C_sw, C, pipeline_depth);

     std::cout << "TEST " << (test_failed ? "FAILED" : "PASSED") << std::endl;

     for (int m = 0; m < NUM_MAT; m++)
     {
          sds_free(A[m]);
          sds_free(B[m]);
          sds_free(C[m]);
          sds_free(C_sw[m]);
     }

     return (test_failed ? -1 : 0);
}
