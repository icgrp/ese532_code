#include "EventTimer.h"
#include "Utilities.h"
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <vector>

static void init_arrays(float *A, float *B) {
    for (int i = 0; i < N; i++) {
      for (int j = 0; j < N; j++) {
        A[i * N + j] = rand() % 512;
        B[i * N + j] = rand() % 512;
      }
    }
}

void mmult_cpu(float *A, float *B, float *C) {
  for (int row = 0; row < N; row++) {
    for (int col = 0; col < N; col++) {
      float result = 0.0;
      for (int k = 0; k < N; k++) {
        result += A[row * N + k] * B[k * N + col];
      }
      C[row * N + col] = result;
    }
  }
}

int main(int argc, char *argv[]) {
  EventTimer timer;
  float *A, *B, *C;
  // matrix size, N x N
  unsigned int N = 1024;

  // pass from the command line if you want
  // to see how long things take
  if (argc == 2) {
    N = atoi(argv[1]);
  }
  A = (float *)malloc(N * N * sizeof(float));
  B = (float *)malloc(N * N * sizeof(float));
  C = (float *)malloc(N * N * sizeof(float));
  if (!A || !B || !C) {
    if (A)
      free(A);
    if (B)
      free(B);
    if (C)
      free(C);
    return 2;
  }

  std::cout << "Running " << N << "x" << N
            << " floating point mmult on cpu..." << std::endl;

  timer.add("Creating arrays: A, B, C");
  init_arrays(A, B);

  timer.add("Running mmult_cpu");
  mmult_cpu(A, B, C);
  store_data("output_cpu.bin", C, N * N * sizeof(float));

  free(A);
  free(B);
  free(C);
  timer.finish();
  std::cout << "--------------- Key execution times ---------------"
            << std::endl;
  timer.print();

  return 0;
}
