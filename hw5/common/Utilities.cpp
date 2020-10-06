#include "Utilities.h"

matrix_type * Create_matrix(void)
{
  matrix_type * Matrix = static_cast<matrix_type *>(
      malloc(MATRIX_WIDTH * MATRIX_WIDTH * sizeof(matrix_type)));
  if (Matrix == NULL)
  {
    std::cerr << "Could not allocate matrix." << std::endl;
    exit (EXIT_FAILURE);
  }
  return Matrix;
}

void Destroy_matrix(matrix_type * Matrix)
{
  free(Matrix);
}

void Randomize_matrix(matrix_type * Matrix)
{
  for (int Y = 0; Y < MATRIX_WIDTH; Y++)
    for (int X = 0; X < MATRIX_WIDTH; X++)
      Matrix[Y * MATRIX_WIDTH + X] = rand();
}

bool Compare_matrices(const matrix_type *Matrix_1,
                      const matrix_type *Matrix_2)
{
  bool Equal = true;
  for (int Y = 0; Y < MATRIX_WIDTH; Y++)
    for (int X = 0; X < MATRIX_WIDTH; X++)
      if ((Matrix_1[Y * MATRIX_WIDTH + X] - Matrix_2[Y * MATRIX_WIDTH + X]) / Matrix_1[Y * MATRIX_WIDTH + X] > 0.01 ||
          (Matrix_1[Y * MATRIX_WIDTH + X] - Matrix_2[Y * MATRIX_WIDTH + X]) / Matrix_1[Y * MATRIX_WIDTH + X] < -0.01)
      //if (Matrix_1[Y * MATRIX_WIDTH + X] != Matrix_2[Y * MATRIX_WIDTH + X])
      {
        std::cout << Matrix_1[Y * MATRIX_WIDTH + X] << "!=" << Matrix_2[Y * MATRIX_WIDTH + X] << std::endl;
        Equal = false;
      }
  return Equal;
}