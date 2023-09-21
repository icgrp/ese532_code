#define HEIGHT (540)
#define WIDTH (960)
#include <stdio.h>


#ifdef VECTORIZED
#include <arm_neon.h>
void Scale(const unsigned char *Input, unsigned char *Output)
{
  // Write your NEON accelerated version of Scale here.
}
#else
void Scale(const unsigned char *Input, unsigned char *Output)
{
  for(int Y = 0; Y < HEIGHT; Y += 2)
  {
    for(int X = 0; X < WIDTH; X += 2)
    {
      Output[(Y / 2) * (WIDTH / 2) + (X / 2)] = Input[(Y * WIDTH) + X];
    }
  }
}
#endif