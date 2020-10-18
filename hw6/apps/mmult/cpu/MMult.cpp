#include <stdio.h>
#include <stdlib.h>

#include "mmult_accel.h"

void mmult(float A[CHUNKS * N * N], float B[CHUNKS * N * N],
           float C[CHUNKS * N * N]) {
  float A_tmp[N][N], B_tmp[N][N];
  for (int c = 0; c < CHUNKS; c++) {
    for (int i = 0; i < N; i++) {
      for (int j = 0; j < N; j++) {
        A_tmp[i][j] = A[c * N * N + i * N + j];
        B_tmp[i][j] = B[c * N * N + i * N + j];
      }
    }
  }

  for (int c = 0; c < CHUNKS; c++) {
    for (int i = 0; i < N; i++) {
      for (int j = 0; j < N; j++) {
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
