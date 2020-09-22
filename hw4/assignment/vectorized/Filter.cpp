#include <stdlib.h>
#include "App.h"

#define INPUT_HEIGHT (270)
#define INPUT_WIDTH (480)

#define FILTER_LENGTH (7)

#define OUTPUT_HEIGHT (INPUT_HEIGHT - (FILTER_LENGTH - 1))
#define OUTPUT_WIDTH (INPUT_WIDTH - (FILTER_LENGTH - 1))

void Filter(const unsigned char *Input, unsigned char *Output)
{
  unsigned char *Temp = (unsigned char *)malloc(INPUT_HEIGHT * OUTPUT_WIDTH);

  Filter_horizontal(Input, Temp);
  Filter_vertical(Temp, Output);

  free(Temp);
}
