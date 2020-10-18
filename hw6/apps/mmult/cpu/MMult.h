
#ifndef MMULT_H_
#define MMULT_H_

#define N 32
#define CHUNKS 1

#define NUM_MAT 4
#define NUM_TESTS 256

#define PIPELINE_DEPTH_MIN 1
#define PIPELINE_DEPTH_MAX 4
#define PIPELINE_DEPTH_DEFAULT 4

void mmult(float A[CHUNKS * N * N], float B[CHUNKS * N * N],
           float C[CHUNKS * N * N]);

#endif
