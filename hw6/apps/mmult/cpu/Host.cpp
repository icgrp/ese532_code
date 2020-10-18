#include "EventTimer.h"
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <vector>

#define N 32
#define CHUNKS 1

#define NUM_MAT 4
#define NUM_TESTS 8192

#define PIPELINE_DEPTH_MIN 1
#define PIPELINE_DEPTH_MAX 4
#define PIPELINE_DEPTH_DEFAULT 4

static void init_arrays(float *A[NUM_MAT], float *B[NUM_MAT],
                        float *C[NUM_MAT]) {
  for (int m = 0; m < NUM_MAT; m++) {
    for (int c = 0; c < CHUNKS; c++) {
      for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
          A[m][c * N * N + i * N + j] = 1 + i * N + j;
          B[m][c * N * N + i * N + j] = rand() % (N * N);
          C[m][c * N * N + i * N + j] = 0.0;
        }
      }
    }
  }
}

void mmult_cpu(float *A, float *B, float *C) {
  for (int c = 0; c < CHUNKS; c++)
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

void mmult_test(float *A[NUM_MAT], float *B[NUM_MAT],
               float *C[NUM_MAT]) {
  std::cout << "Running " << CHUNKS << "x" << NUM_TESTS << " iterations of "
            << N << "x" << N << " floating point mmult..."
            << std::endl;

  EventTimer timer;
  
  timer.add("Creating arrays: A, B, C");
  init_arrays(A, B, C);
  
  timer.add("NUM_TESTS iterations of mmult_cpu");
  for (int i = 0; i < NUM_TESTS; i++) {
    mmult_cpu(A[i % NUM_MAT], B[i % NUM_MAT], C[i % NUM_MAT]);
  }
  
  timer.finish();
  std::cout << "--------------- Key execution times ---------------" << std::endl;
  timer.print();
}

int main(int argc, char *argv[]) {
  EventTimer timer;
  timer.add("Main function");
  float *A[NUM_MAT], *B[NUM_MAT], *C[NUM_MAT];

  for (int m = 0; m < NUM_MAT; m++) {
    A[m] = (float *)malloc(CHUNKS * N * N * sizeof(float));
    B[m] = (float *)malloc(CHUNKS * N * N * sizeof(float));
    C[m] = (float *)malloc(CHUNKS * N * N * sizeof(float));

    if (!A[m] || !B[m] || !C[m]) {
      if (A[m])
        free(A[m]);
      if (B[m])
        free(B[m]);
      if (C[m])
        free(C[m]);
      return 2;
    }
  }
  
  mmult_test(A, B, C);

  for (int m = 0; m < NUM_MAT; m++) {
    free(A[m]);
    free(B[m]);
    free(C[m]);
  }
  timer.finish();
  std::cout << "--------------- Total runtime ---------------" << std::endl;
  timer.print();

  return 0;
}
