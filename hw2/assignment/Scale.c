#define HEIGHT (3000)
#define WIDTH (4000)

void Scale(const unsigned char * Input, unsigned char * Output)
{
  for (int Y = 0; Y < HEIGHT; Y += 2)
    for (int X = 0; X < WIDTH; X += 2)
      Output[(Y / 2) * WIDTH / 2 + (X / 2)] = Input[Y * WIDTH + X];
}
