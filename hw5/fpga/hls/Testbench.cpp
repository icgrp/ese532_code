#include <iostream>
#include <cstdlib>
#include "MatrixMultiplication.h"

matrix_type * Create_matrix(void)
{
  matrix_type * Matrix = static_cast<matrix_type *>(
#ifdef __SDSCC__
      sds_alloc(MATRIX_WIDTH * MATRIX_WIDTH * sizeof(matrix_type)));
#else
      malloc(MATRIX_WIDTH * MATRIX_WIDTH * sizeof(matrix_type)));
#endif
  if (Matrix == NULL)
  {
    std::cerr << "Could not allocate matrix." << std::endl;
    exit (EXIT_FAILURE);
  }

  return Matrix;
}

void Destroy_matrix(matrix_type * Matrix)
{
#ifdef __SDSCC__
  sds_free(Matrix);
#else
  free(Matrix);
#endif
}

void Randomize_matrix(matrix_type * Matrix)
{
  for (int Y = 0; Y < MATRIX_WIDTH; Y++)
    for (int X = 0; X < MATRIX_WIDTH; X++)
      Matrix[Y * MATRIX_WIDTH + X] = rand();
}

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

void Show_matrix(const matrix_type * Matrix)
{
  for (int Y = 0; Y < MATRIX_WIDTH; Y++)
  {
    for (int X = 0; X < MATRIX_WIDTH; X++)
    {
      if (X > 0) std::cout << ' ';
      std::cout << Matrix[Y * MATRIX_WIDTH + X] << std::endl;
    }
    std::cout << std::endl;
  }
}

bool Compare_matrices(const matrix_type * Matrix_1,
    const matrix_type * Matrix_2)
{
  bool Equal = true;
  for (int Y = 0; Y < MATRIX_WIDTH; Y++)
    for (int X = 0; X < MATRIX_WIDTH; X++)
      if ((Matrix_1[Y * MATRIX_WIDTH + X]-Matrix_2[Y * MATRIX_WIDTH + X])/Matrix_1[Y * MATRIX_WIDTH + X] > 0.01
	  	  ||
		  (Matrix_1[Y * MATRIX_WIDTH + X]-Matrix_2[Y * MATRIX_WIDTH + X])/Matrix_1[Y * MATRIX_WIDTH + X] < -0.01)
      //if (Matrix_1[Y * MATRIX_WIDTH + X] != Matrix_2[Y * MATRIX_WIDTH + X])
      {
    	std::cout << Matrix_1[Y * MATRIX_WIDTH + X] << "!=" << Matrix_2[Y * MATRIX_WIDTH + X] <<std::endl;
        Equal = false;
      }
  return Equal;
}

int main()
{
  matrix_type * Input_1 = Create_matrix();
  matrix_type * Input_2 = Create_matrix();
  matrix_type * Output_SW = Create_matrix();
  matrix_type * Output_HW = Create_matrix();

  Randomize_matrix(Input_1);
  Randomize_matrix(Input_2);

#ifdef __SDSCC__
  unsigned long long Start_time_SW = sds_clock_counter();
#endif
  Multiply_SW(Input_1, Input_2, Output_SW);
#ifdef __SDSCC__
  unsigned long long End_time_SW = sds_clock_counter();
#endif

#ifdef __SDSCC__
  unsigned long long Start_time_HW = sds_clock_counter();
#endif
  Multiply_HW(Input_1, Input_2, Output_HW);
#ifdef __SDSCC__
  unsigned long long End_time_HW = sds_clock_counter();
#endif

/*
  std::cout << "Input 1\n";
  Show_matrix(Input_1);
  std::cout << "Input 2\n";
  Show_matrix(Input_2);
  std::cout << "Output of software implementation\n";
  Show_matrix(Output_SW);
  std::cout << "Output of hardware implementation\n";
  Show_matrix(Output_HW);
*/

  bool Equal = Compare_matrices(Output_SW, Output_HW);

  Destroy_matrix(Input_1);
  Destroy_matrix(Input_2);
  Destroy_matrix(Output_SW);
  Destroy_matrix(Output_HW);

#ifdef __SDSCC__
  unsigned long long Duration_SW = End_time_SW - Start_time_SW;
  unsigned long long Duration_HW = End_time_HW - Start_time_HW;

  std::cout << "The baseline took " << (float)Duration_SW * 1000000 / sds_clock_frequency() << " us.\n";
  std::cout << "The optimized version took " << (float) Duration_HW *1000000 / sds_clock_frequency() << " us.\n";
  std::cout << "Speedup: " << (double) Duration_SW / Duration_HW << '\n';
#endif


  std::cout << "TEST " << (Equal ? "PASSED" : "FAILED") << std::endl;

  return Equal ? 0 : 1;
}
