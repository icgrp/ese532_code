#ifndef SRC_UTILITIES_
#define SRC_UTILITIES_

#include <iostream>
#include <cstdlib>
#include "Constants.h"
#include "Stopwatch.h"

matrix_type *Create_matrix(void);
void Destroy_matrix(matrix_type *Matrix);
void Randomize_matrix(matrix_type *Matrix);
bool Compare_matrices(const matrix_type *Matrix_1,
                      const matrix_type *Matrix_2);

#endif