#include "Encrypt.h"

#ifdef __SDSCC__
#include <sds_lib.h>
#else
#include <time.h>
#endif

#include <stdio.h>
#include <stdlib.h>

uint32_t * Create_block(int Length)
{
  uint32_t * Block = (uint32_t *) malloc(Length * sizeof(uint32_t));
  if (Block == NULL)
  {
	fputs("Could not allocate block.\n", stderr);
    exit(EXIT_FAILURE);
  }

  return Block;
}

void Destroy_block(uint32_t * Block)
{
  free(Block);
}

void Randomize_block(uint32_t * Block, int Length)
{
  int i;
  for (i = 0; i < Length; i++)
    Block[i] = rand();
}

static int Table[] = {4, 0, 10, 12, 17, 20, 22, 25, 15, 21, 28, 24, 9, 7, 3, 30, 8,
		              14, 6, 29, 16, 5, 23, 2, 26, 13, 31, 11, 27, 19, 1, 18};

void Encrypt_SW(const uint32_t * Input, uint32_t Key, uint32_t * Output,
		        int Length)
{
  int i, j;
  for (i = 0; i < Length; i++)
  {
    uint32_t Input_value = Input[i];

    uint32_t Permuted_value = 0;
    for (j = 0; j < 32; j++)
    {
      Permuted_value |= (Input_value & 1) << Table[j];
      Input_value >>= 1;
    }

    Output[i] = Permuted_value ^ Key;
  }
}

void Show_block(const uint32_t * Block, int Length)
{
  int i;
  for (i = 0; i < Length; i++)
  {
    if (i > 0) putchar(' ');
    printf("%u", Block[i]);
  }
  putchar('\n');
}

int Compare_blocks(const uint32_t * Block_1,
    const uint32_t * Block_2, int Length)
{
	  int i;
  int Equal = 1;
  for (i = 0; i < Length; i++)
    if (Block_1[i] != Block_2[i])
      Equal = 0;
  return Equal;
}

int main()
{
#ifdef __SDSCC__
  srand((unsigned) sds_clock_counter());
#else
  srand(time(NULL));
#endif

  int Length = (MAX_SIZE / 2) + (rand() % (MAX_SIZE / 2)) + 1;
  int Key = rand();

  uint32_t * Input = Create_block(MAX_SIZE);
  uint32_t * Output_SW = Create_block(MAX_SIZE);
  uint32_t * Output_HW = Create_block(MAX_SIZE);

  Randomize_block(Input, Length);

#ifdef __SDSCC__
  unsigned long long Start_time_SW = sds_clock_counter();
#endif
  Encrypt_SW(Input, Key, Output_SW, Length);
#ifdef __SDSCC__
  unsigned long long End_time_SW = sds_clock_counter();
#endif

#ifdef __SDSCC__
  unsigned long long Start_time_HW = sds_clock_counter();
#endif
  Encrypt_HW(Input, Key, Output_HW, Length);
#ifdef __SDSCC__
  unsigned long long End_time_HW = sds_clock_counter();
#endif

/*
  printf("Input\n");
  Show_block(Input, Length);
  printf("Output of software implementation\n");
  Show_block(Output_SW, Length);
  printf("Output of hardware implementation\n");
  Show_block(Output_HW, Length);
*/

  int Equal = Compare_blocks(Output_SW, Output_HW, Length);

  Destroy_block(Input);
  Destroy_block(Output_SW);
  Destroy_block(Output_HW);

#ifdef __SDSCC__
  unsigned long long Duration_SW = End_time_SW - Start_time_SW;
  unsigned long long Duration_HW = End_time_HW - Start_time_HW;

  printf("The baseline took %llu cycles.\n", Duration_SW);
  printf("The optimized version took %llu cycles.\n", Duration_HW);
  printf("Speedup: %f\n", (double) Duration_SW / Duration_HW);
#endif

  if (Equal)
    printf("TEST PASSED\n");
  else
    printf("TEST FAILED\n");

  return Equal ? 0 : 1;
}
