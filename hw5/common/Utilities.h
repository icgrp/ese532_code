#ifndef SRC_UTILITIES_
#define SRC_UTILITIES_

#include <iostream>
#include <cstdlib>
#include <CL/cl2.hpp>
#include "Constants.h"
#include "EventTimer.h"

matrix_type *Create_matrix(void);
void Destroy_matrix(matrix_type *Matrix);
void Randomize_matrix(matrix_type *Matrix);
bool Compare_matrices(const matrix_type *Matrix_1,
                      const matrix_type *Matrix_2);
void multiply_gold(const matrix_type Input_1[MATRIX_WIDTH * MATRIX_WIDTH],
                 const matrix_type Input_2[MATRIX_WIDTH * MATRIX_WIDTH],
		         matrix_type Output[MATRIX_WIDTH * MATRIX_WIDTH]);
std::vector<cl::Device> get_xilinx_devices();
char* read_binary_file(const std::string &xclbin_file_name, unsigned &nb);

#endif