#include "EventTimer.h"
#include "Utilities.h"
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <vector>

constexpr int N = 32;

constexpr int PIPELINE_DEPTH_MIN = 1;
constexpr int PIPELINE_DEPTH_MAX = 4;
constexpr int PIPELINE_DEPTH_DEFAULT = 4;

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

int main(int argc, char *argv[]) {
  EventTimer timer;
  float *A, *B, *C;
  unsigned int num_tests = 8192;

  if (argc == 2) {
    num_tests = atoi(argv[1]);
  }
  A = (float *)malloc(num_tests * N * N * sizeof(float));
  B = (float *)malloc(num_tests * N * N * sizeof(float));
  C = (float *)malloc(num_tests * N * N * sizeof(float));
  if (!A || !B || !C) {
    if (A)
      free(A);
    if (B)
      free(B);
    if (C)
      free(C);
    return 2;
  }

  std::cout << "Running " << num_tests << " " << N << "x" << N
            << " floating point mmult on cpu..." << std::endl;

  timer.add("Creating arrays: A, B, C");
  init_arrays(A, B, num_tests);

  timer.add("Running mmult_cpu");
  mmult_cpu(A, B, C, num_tests);
  store_data("output_cpu.bin", C, num_tests * N * N * sizeof(float));

  free(A);
  free(B);
  free(C);
  timer.finish();
  std::cout << "--------------- Key execution times ---------------"
            << std::endl;
  timer.print();

  return 0;
}
