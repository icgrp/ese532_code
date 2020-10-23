#include "EventTimer.h"
#include "Utilities.h"
#include "MMult.h"
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <vector>

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

int main(int argc, char *argv[]) {
  EventTimer timer1, timer2;
  timer1.add("Main program");

  std::cout << "Running " << CHUNKS << "x" <<NUM_TESTS << " iterations of " << N << "x" << N
               << " floating point mmult..." << std::endl;

  timer2.add("Allocating arrays");
  size_t elements_per_iteration = CHUNKS * N * N;
  size_t bytes_per_iteration = elements_per_iteration * sizeof(float);
  float *A[NUM_MAT], *B[NUM_MAT], *C[NUM_MAT];

  for (int m = 0; m < NUM_MAT; m++) {
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
  }
  init_arrays(A, B);

  timer2.add("Running mmult_cpu");
  for (int i = 0; i < NUM_TESTS; i++) {
    mmult_cpu(A[i%NUM_MAT], B[i%NUM_MAT], C[i%NUM_MAT]);
  }

  timer2.add("Writing output to output_cpu.bin");
  FILE *file = fopen("output_cpu.bin", "wb");
  for (int m = 0; m < NUM_MAT; m++) {
    fwrite(C[m], 1, bytes_per_iteration, file);
    free(A[m]);
    free(B[m]);
    free(C[m]);
  }
  fclose(file);
  
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
