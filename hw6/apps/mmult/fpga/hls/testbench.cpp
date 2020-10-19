#include "MMult.h"
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <vector>


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

void mmult_cpu(float *A, float *B, float *C, unsigned int num_tests) {
  for (int c = 0; c < num_tests; c++)
    for (int row = 0; row < N; row++) {
      for (int col = 0; col < N; col++) {
        float result = 0.0;
        for (int k = 0; k < N; k++) {
          result += A[c * N * N + row * N + k] * B[c * N * N + k * N + col];
        }
        C[c * N * N + row * N + col] = result;
      }
    }
}

static int result_check(float *c_fpga, float *c_cpu, unsigned int num_tests) {
  for (int i = 0; i < num_tests * N * N; i++) {
    if (c_cpu[i] != c_fpga[i]) {
      std::cout << "Mismatch: data index=" << i << " d=" << c_cpu[i]
                << ", dout=" << c_fpga[i] << std::endl;
      return 1;
    }
  }
  return 0;
}

int main(int argc, char *argv[]) {
  float *A, *B, *C_cpu, *C_fpga;
  unsigned int num_tests = 8192;

  A = (float *)malloc(num_tests * N * N * sizeof(float));
  B = (float *)malloc(num_tests * N * N * sizeof(float));
  C_cpu = (float *)malloc(num_tests * N * N * sizeof(float));
  C_fpga = (float *)malloc(num_tests * N * N * sizeof(float));
  if (!A || !B || !C_cpu || !C_fpga) {
    if (A)
      free(A);
    if (B)
      free(B);
    if (C_cpu)
      free(C_cpu);
    if (C_fpga)
      free(C_fpga);
    return 2;
  }

  init_arrays(A, B, num_tests);
  mmult_cpu(A, B, C_cpu, num_tests);
  mmult_fpga(A, B, C_fpga, num_tests);
  int equal = result_check(C_fpga, C_cpu, num_tests);
  std::cout << "TEST " << (equal ? "PASSED" : "FAILED") << std::endl;

  free(A);
  free(B);
  free(C_cpu);
  free(C_fpga);

  return equal ? 0 : 1;
}
