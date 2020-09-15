#ifndef SRC_APP_H_
#define SRC_APP_H_

#define HEIGHT (540)
#define WIDTH (960)

void Scale(const unsigned char * Input, unsigned char * Output, int Y_Start_Idx, int Y_End_Idx, int i);
void Filter(const unsigned char * Input, unsigned char * Output);
void Differentiate(const unsigned char * Input, unsigned char * Output);
int Compress(const unsigned char * Input, unsigned char * Output);

#endif

