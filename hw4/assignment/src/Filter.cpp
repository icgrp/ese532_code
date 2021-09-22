#include <stdlib.h>

#define INPUT_HEIGHT (270)
#define INPUT_WIDTH (480)

#define FILTER_LENGTH (7)

#define OUTPUT_HEIGHT (INPUT_HEIGHT - (FILTER_LENGTH - 1))
#define OUTPUT_WIDTH (INPUT_WIDTH - (FILTER_LENGTH - 1))

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

#ifdef VECTORIZED
#include <arm_neon.h>
void Filter_vertical(const unsigned char *Input, unsigned char *Output)
{
  // modify the following code
  uint16x8_t Coef0 = vdupq_n_u16(Coefficients[0]);
  uint16x8_t Coef1 = vdupq_n_u16(Coefficients[1]);
  uint16x8_t Coef2 = vdupq_n_u16(Coefficients[2]);
  uint16x8_t Coef3 = vdupq_n_u16(Coefficients[3]);
  for (int X = 0; X < OUTPUT_WIDTH; X += 16)
  {
    uint8x16_t Data0 = vld1q_u8(Input + 0 * OUTPUT_WIDTH + X);
   	uint8x16_t Data1 = vld1q_u8(Input + 1 * OUTPUT_WIDTH + X);
   	uint8x16_t Data2 = vld1q_u8(Input + 2 * OUTPUT_WIDTH + X);
   	uint8x16_t Data3 = vld1q_u8(Input + 3 * OUTPUT_WIDTH + X);
   	uint8x16_t Data4 = vld1q_u8(Input + 4 * OUTPUT_WIDTH + X);
   	uint8x16_t Data5 = vld1q_u8(Input + 5 * OUTPUT_WIDTH + X);
    for (int Y = 0; Y < OUTPUT_HEIGHT; Y++)
    {
   	  uint8x16_t Data6 = vld1q_u8(Input + (Y + 6) * OUTPUT_WIDTH + X);
   	  uint16x8_t Sum0H = vaddl_u8(vget_high_u8(Data0), vget_high_u8(Data6));
   	  uint16x8_t Sum1H = vaddl_u8(vget_high_u8(Data1), vget_high_u8(Data5));
   	  uint16x8_t Sum2H = vaddl_u8(vget_high_u8(Data2), vget_high_u8(Data4));
   	  uint16x8_t SumH = vmulq_u16(Sum0H, Coef0);
   	  SumH = vmlaq_u16(SumH, Sum1H, Coef1);
   	  SumH = vmlaq_u16(SumH, Sum2H, Coef2);
   	  SumH = vmlaq_u16(SumH, vmovl_u8(vget_high_u8(Data3)), Coef3);
   	  SumH = vshrq_n_u16(SumH, 8);
   	  uint8x8_t ResultH = vmovn_u16(SumH);
   	  uint16x8_t Sum0L = vaddl_u8(vget_low_u8(Data0), vget_low_u8(Data6));
   	  uint16x8_t Sum1L = vaddl_u8(vget_low_u8(Data1), vget_low_u8(Data5));
   	  uint16x8_t Sum2L = vaddl_u8(vget_low_u8(Data2), vget_low_u8(Data4));
   	  uint16x8_t SumL = vmulq_u16(Sum0L, Coef0);
   	  SumL = vmlaq_u16(SumL, Sum1L, Coef1);
   	  SumL = vmlaq_u16(SumL, Sum2L, Coef2);
   	  SumL = vmlaq_u16(SumL, vmovl_u8(vget_low_u8(Data3)), Coef3);
   	  SumL = vshrq_n_u16(SumL, 8);
   	  uint8x8_t ResultL = vmovn_u16(SumL);
   	  vst1q_u8(Output + Y * OUTPUT_WIDTH + X, vcombine_u8(ResultL, ResultH));
   	  Data0 = Data1;
   	  Data1 = Data2;
   	  Data2 = Data3;
   	  Data3 = Data4;
   	  Data4 = Data5;
   	  Data5 = Data6;
    }
  }
}
#else
void Filter_vertical(const unsigned char *Input, unsigned char *Output)
{
  for (int Y = 0; Y < OUTPUT_HEIGHT; Y++)
    for (int X = 0; X < OUTPUT_WIDTH; X++)
    {
      unsigned int Sum = 0;
      for (int i = 0; i < FILTER_LENGTH; i++)
        Sum += Coefficients[i] * Input[(Y + i) * OUTPUT_WIDTH + X];
      Output[Y * OUTPUT_WIDTH + X] = Sum >> 8;
    }
}
#endif

void Filter(const unsigned char *Input, unsigned char *Output)
{
  unsigned char *Temp = (unsigned char *)malloc(INPUT_HEIGHT * OUTPUT_WIDTH);

  Filter_horizontal(Input, Temp);
  Filter_vertical(Temp, Output);

  free(Temp);
}
