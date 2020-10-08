#include <iostream>
#include <cstdlib>
#include <chrono>
#include "MatrixMultiplication.h"

class stopwatch
{
public:
  double total_time, calls;
  std::chrono::time_point<std::chrono::high_resolution_clock> start_time, end_time;
  stopwatch() : total_time(0), calls(0){};

  inline void reset()
  {
    total_time = 0;
    calls = 0;
  }

  inline void start()
  {
    start_time = std::chrono::high_resolution_clock::now();
    calls++;
  };

  inline void stop()
  {
    end_time = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count();
    total_time += static_cast<double>(elapsed);
  };

  // return latency in ns
  inline double latency()
  {
    return total_time;
  };

  // return latency in ns
  inline double avg_latency()
  {
    return (total_time / calls);
  };
};


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

void multiply_gold(const matrix_type Input_1[MATRIX_WIDTH * MATRIX_WIDTH],
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

  multiply_gold(Input_1, Input_2, Output_SW);
  mmult(Input_1, Input_2, Output_HW);

  bool Equal = Compare_matrices(Output_SW, Output_HW);

  Destroy_matrix(Input_1);
  Destroy_matrix(Input_2);
  Destroy_matrix(Output_SW);
  Destroy_matrix(Output_HW);

  std::cout << "TEST " << (Equal ? "PASSED" : "FAILED") << std::endl;

  return Equal ? 0 : 1;
}
