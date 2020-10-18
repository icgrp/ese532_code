#include "Encrypt.h"

static int Table[] = {4, 0, 10, 12, 17, 20, 22, 25, 15, 21, 28, 24, 9, 7, 3, 30, 8,
		              14, 6, 29, 16, 5, 23, 2, 26, 13, 31, 11, 27, 19, 1, 18};

void Encrypt_HW(const uint32_t * Input, uint32_t Key,
                uint32_t * Output, int Length)
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
