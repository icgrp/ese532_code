#ifndef SRC_APP_H_
#define SRC_APP_H_

void Scale(const unsigned char * Input, unsigned char * Output);
void Filter(const unsigned char * Input, unsigned char * Output);
void Differentiate(const unsigned char * Input, unsigned char * Output);
int Compress(const unsigned char * Input, unsigned char * Output);
void Filter_horizontal(const unsigned char * Input, unsigned char * Output);
#endif

