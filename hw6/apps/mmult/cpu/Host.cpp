#include "EventTimer.h"
#include "MMult.h"
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <vector>

static void init_arrays(float *A[NUM_MAT], float *B[NUM_MAT],
                        float *C_sw[NUM_MAT], float *C[NUM_MAT]) {
  for (int m = 0; m < NUM_MAT; m++) {
    for (int c = 0; c < CHUNKS; c++) {
      for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
          A[m][c * N * N + i * N + j] = 1 + i * N + j;
          B[m][c * N * N + i * N + j] = rand() % (N * N);
          C_sw[m][c * N * N + i * N + j] = 0.0;
          C[m][c * N * N + i * N + j] = 0.0;
        }
      }
    }
  }
}

void mmult_golden(float *A, float *B, float *C) {
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

static int result_check(float *C[NUM_MAT], float *C_sw[NUM_MAT]) {
  for (int m = 0; m < NUM_MAT; m++) {
    for (int i = 0; i < CHUNKS * N * N; i++) {
      if (C_sw[m][i] != C[m][i]) {
        std::cout << "Mismatch: data index=" << i << " d=" << C_sw[m][i]
                  << ", dout=" << C[m][i] << std::endl;
        return 1;
      }
    }
  }
  return 0;
}

int mmult_test(float *A[NUM_MAT], float *B[NUM_MAT], float *C_sw[NUM_MAT],
               float *C[NUM_MAT], int pipeline_depth) {
  std::cout << "Running " << CHUNKS << "x" << NUM_TESTS << " iterations of "
            << N << "x" << N << " task pipelined floating point mmult..."
            << std::endl;

  EventTimer timer;

  init_arrays(A, B, C_sw, C);

  for (int i = 0; i < NUM_TESTS; i++) {
    mmult_golden(A[i % NUM_MAT], B[i % NUM_MAT], C_sw[i % NUM_MAT]);
  }
  
  timer.add("mmult on cpu");
  for (int i = 0; i < NUM_TESTS; i++) {
    mmult_accel(A[i % NUM_MAT], B[i % NUM_MAT], C[i % NUM_MAT]);
  }
  timer.finish()
  std::cout << "--------------- Key execution times ---------------" << std::endl;
  timer.print();

  int result = 0;
  for (int i = 0; !result && i < NUM_MAT; i++)
    result = result_check(C, C_sw);
  return result;
}

int main(int argc, char *argv[]) {
  int pipeline_depth = PIPELINE_DEPTH_DEFAULT;
  float *A[NUM_MAT], *B[NUM_MAT], *C_sw[NUM_MAT], *C[NUM_MAT];

  // use pipeline depth from command argument, if passed in
  if (argc == 2) {
    pipeline_depth = atoi(argv[1]);
    if (pipeline_depth > PIPELINE_DEPTH_MAX ||
        pipeline_depth < PIPELINE_DEPTH_MIN) {
      std::cout << "Please use pipeline depth from " << PIPELINE_DEPTH_MIN
                << " to " << PIPELINE_DEPTH_MAX << std::endl;
      return 1;
    }
  }
  for (int m = 0; m < NUM_MAT; m++) {
    A[m] = (float *)malloc(CHUNKS * N * N * sizeof(float));
    B[m] = (float *)malloc(CHUNKS * N * N * sizeof(float));
    C[m] = (float *)malloc(CHUNKS * N * N * sizeof(float));
    C_sw[m] = (float *)malloc(CHUNKS * N * N * sizeof(float));

    if (!A[m] || !B[m] || !C[m] || !C_sw[m]) {
      if (A[m])
        free(A[m]);
      if (B[m])
        free(B[m]);
      if (C[m])
        free(C[m]);
      if (C_sw[m])
        free(C_sw[m]);
      return 2;
    }
  }
  int test_failed = mmult_test(A, B, C_sw, C, pipeline_depth);

  std::cout << "TEST " << (test_failed ? "FAILED" : "PASSED") << std::endl;

  for (int m = 0; m < NUM_MAT; m++) {
    free(A[m]);
    free(B[m]);
    free(C[m]);
    free(C_sw[m]);
  }

  return (test_failed ? -1 : 0);
}
