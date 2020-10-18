#include "MMult.h"

/**
 *
 * Design principles to achieve II = 1
 * 1. Stream data into local RAM for inputs (multiple access required)
 * 2. Partition local RAMs into N/2 sub-arrays for fully parallel access
 * (dual-port read)
 * 3. Pipeline the dot-product loop, to fully unroll it
 * 4. Separate multiply-accumulate in inner loop to force two FP operators
 *
 */
void mmult_fpga(float *A, float *B, float *C, unsigned int num_tests) {
#pragma HLS INTERFACE m_axi port = Input_1 bundle = aximm1
#pragma HLS INTERFACE m_axi port = Input_2 bundle = aximm1
#pragma HLS INTERFACE m_axi port = Output bundle = aximm1
  float A_tmp[N][N], B_tmp[N][N];
#pragma HLS array_partition variable = A_tmp block factor = 16 dim = 2
#pragma HLS array_partition variable = B_tmp block factor = 16 dim = 1

  for (int c = 0; c < num_tests; c++) {
    for (int i = 0; i < N; i++) {
      for (int j = 0; j < N; j++) {
#pragma HLS PIPELINE
        A_tmp[i][j] = A[c * N * N + i * N + j];
        B_tmp[i][j] = B[c * N * N + i * N + j];
      }
    }
  }

  for (int c = 0; c < num_tests; c++) {
    for (int i = 0; i < N; i++) {
      for (int j = 0; j < N; j++) {
#pragma HLS PIPELINE
        float result = 0;
        for (int k = 0; k < N; k++) {
          float term = A_tmp[i][k] * B_tmp[k][j];
          result += term;
        }
        C[c * N * N + i * N + j] = result;
      }
    }
  }
}
