#include "App.h"

void Scale(const unsigned char *Input, unsigned char *Output)
{
    for (int Y = 0; Y < INPUT_HEIGHT_SCALE; Y += 2)
        for (int X = 0; X < INPUT_WIDTH_SCALE; X += 2)
            Output[(Y / 2) * INPUT_WIDTH_SCALE / 2 + (X / 2)] = Input[Y * INPUT_WIDTH_SCALE + X];
}
