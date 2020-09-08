#define HEIGHT (1494)
#define WIDTH (1994)

void Differentiate(const unsigned char * Input, unsigned char * Output)
{
  for (int Y = 0; Y < HEIGHT; Y++)
    for (int X = 0; X < WIDTH; X++)
    {
      int Average = 0;
      if (Y > 0 && X > 0)
        Average = (Input[WIDTH * (Y - 1) + X] + Input[WIDTH * Y + X - 1]) / 2;
      else if (Y > 0)
        Average = Input[WIDTH * (Y - 1) + X];
      else if (X > 0)
        Average = Input[WIDTH * Y + X - 1];

      unsigned char Diff = Input[WIDTH * Y + X] - Average;

      Output[Y * WIDTH + X] = Diff;
    }
}
