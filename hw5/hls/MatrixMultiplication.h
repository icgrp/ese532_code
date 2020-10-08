#define MATRIX_WIDTH (64)

typedef float matrix_type;

extern "C"
{
  void Multiply_HW(const matrix_type Input_1[MATRIX_WIDTH * MATRIX_WIDTH],
                   const matrix_type Input_2[MATRIX_WIDTH * MATRIX_WIDTH],
                   matrix_type Output[MATRIX_WIDTH * MATRIX_WIDTH]);
}