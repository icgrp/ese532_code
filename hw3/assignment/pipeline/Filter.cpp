#include <stdlib.h>

#define INPUT_HEIGHT (270)
#define INPUT_WIDTH (480)

#define FILTER_LENGTH (7)

#define OUTPUT_HEIGHT (INPUT_HEIGHT - (FILTER_LENGTH - 1))
#define OUTPUT_WIDTH (INPUT_WIDTH - (FILTER_LENGTH - 1))

#define PIPELINE_PAR (0)

unsigned Coefficients[] = {2, 15, 62, 98, 62, 15, 2};

void Filter_horizontal(const unsigned char *Input, unsigned char *Output)
{
  for (int Y = 0; Y < INPUT_HEIGHT; Y++)
    for (int X = 0; X < OUTPUT_WIDTH; X++)
    {
      unsigned int Sum = 0;
      for (int i = 0; i < FILTER_LENGTH; i++)
        Sum += Coefficients[i] * Input[Y * INPUT_WIDTH + X + i];
      Output[Y * OUTPUT_WIDTH + X] = Sum >> 8;
    }
}

void Filter_vertical_core1(const unsigned char *Input, unsigned char *Output)
{
  for (int Y = 0; Y < OUTPUT_HEIGHT; Y++)
    for (int X = 0; X < PIPELINE_PAR; X++)
    {
      unsigned int Sum = 0;
      for (int i = 0; i < FILTER_LENGTH; i++)
        Sum += Coefficients[i] * Input[(Y + i) * OUTPUT_WIDTH + X];
      Output[Y * OUTPUT_WIDTH + X] = Sum >> 8;
    }

  for (int Y = 0; Y < INPUT_HEIGHT; Y++)
    for (int X = PIPELINE_PAR; X < OUTPUT_WIDTH; X++)
    {
      Output[Y * OUTPUT_WIDTH + X] = Input[Y * OUTPUT_WIDTH + X];
    }
}

void Filter_vertical_core0(const unsigned char *Input, unsigned char *Output)
{
  for (int Y = 0; Y < OUTPUT_HEIGHT; Y++)
    for (int X = 0; X < PIPELINE_PAR; X++)
    {
      Output[Y * OUTPUT_WIDTH + X] = Input[Y * OUTPUT_WIDTH + X];
    }

  for (int Y = 0; Y < OUTPUT_HEIGHT; Y++)
    for (int X = PIPELINE_PAR; X < OUTPUT_WIDTH; X++)
    {
      unsigned int Sum = 0;
      for (int i = 0; i < FILTER_LENGTH; i++)
        Sum += Coefficients[i] * Input[(Y + i) * OUTPUT_WIDTH + X];
      Output[Y * OUTPUT_WIDTH + X] = Sum >> 8;
    }
}

void Filter_core_0(const unsigned char *Input, unsigned char *Output)
{
  Filter_vertical_core0(Input, Output);
}

void Filter_core_1(const unsigned char *Input, unsigned char *Output)
{
  unsigned char *Temp = (unsigned char *)malloc(INPUT_HEIGHT * OUTPUT_WIDTH);

  Filter_horizontal(Input, Temp);
  Filter_vertical_core1(Temp, Output);

  free(Temp);
}
