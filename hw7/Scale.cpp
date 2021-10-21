#include "Pipeline.h"

void Scale_SW(const unsigned char * Input,
	          unsigned char * Output)
{
  for (int Y = 0; Y < INPUT_FRAME_HEIGHT; Y += 2)
    for (int X = 0; X < INPUT_FRAME_WIDTH; X += 2)
      Output[(Y / 2) * SCALED_FRAME_WIDTH + (X / 2)] = Input[Y * INPUT_FRAME_WIDTH + X];
}

