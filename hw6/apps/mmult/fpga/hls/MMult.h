
#ifndef MMULT_ACC_H_
#define MMULT_ACC_H_

constexpr int N = 32;

constexpr int PIPELINE_DEPTH_MIN = 1;
constexpr int PIPELINE_DEPTH_MAX = 4;
constexpr int PIPELINE_DEPTH_DEFAULT = 4;

/**
 * Design principles to achieve best performance
 *
 * 1. Declare secquential access to stream data into accelerator via a hardware
 * FIFO interface.  Otherwise, the default RAM interface requires all data to
 * arrive before starting HLS accelerator
 */
extern "C" {
void mmult_fpga(float *A, float *B, float *C, unsigned int num_tests);
}
#endif /* MMULT_ACC_H_ */
