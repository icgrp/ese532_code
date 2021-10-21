#include "Pipeline.h"

void Differentiate_SW(const unsigned char * Input, unsigned char * Output)
{
  for (int Y = 0; Y < OUTPUT_FRAME_HEIGHT; Y++)
    for (int X = 0; X < OUTPUT_FRAME_WIDTH; X++)
    {
      int Average = 0;
      if (Y > 0 && X > 0)
        Average = (Input[OUTPUT_FRAME_WIDTH * (Y - 1) + X] + Input[OUTPUT_FRAME_WIDTH * Y + X - 1]) / 2;
      else if (Y > 0)
        Average = Input[OUTPUT_FRAME_WIDTH * (Y - 1) + X];
      else if (X > 0)
        Average = Input[OUTPUT_FRAME_WIDTH * Y + X - 1];

      unsigned char Diff = Input[OUTPUT_FRAME_WIDTH * Y + X] - Average;

      Output[Y * OUTPUT_FRAME_WIDTH + X] = Diff;
    }
}

