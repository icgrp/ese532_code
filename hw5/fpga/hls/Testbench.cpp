#include "Utilities.h"

void Multiply_SW(const matrix_type Input_1[MATRIX_WIDTH * MATRIX_WIDTH],
                 const matrix_type Input_2[MATRIX_WIDTH * MATRIX_WIDTH],
                 matrix_type Output[MATRIX_WIDTH * MATRIX_WIDTH])
{
  for (int i = 0; i < MATRIX_WIDTH; i++)
    for (int j = 0; j < MATRIX_WIDTH; j++)
    {
      matrix_type Result = 0;
      for (int k = 0; k < MATRIX_WIDTH; k++)
        Result += Input_1[i * MATRIX_WIDTH + k] * Input_2[k * MATRIX_WIDTH + j];
      Output[i * MATRIX_WIDTH + j] = Result;
    }
}

int main()
{
  matrix_type *Input_1 = Create_matrix();
  matrix_type *Input_2 = Create_matrix();
  matrix_type *Output_SW = Create_matrix();
  matrix_type *Output_HW = Create_matrix();

  Randomize_matrix(Input_1);
  Randomize_matrix(Input_2);

  Multiply_SW(Input_1, Input_2, Output_SW);
  Multiply_HW(Input_1, Input_2, Output_HW);

  bool Equal = Compare_matrices(Output_SW, Output_HW);

  Destroy_matrix(Input_1);
  Destroy_matrix(Input_2);
  Destroy_matrix(Output_SW);
  Destroy_matrix(Output_HW);

  std::cout << "TEST " << (Equal ? "PASSED" : "FAILED") << std::endl;

  return Equal ? 0 : 1;
}
