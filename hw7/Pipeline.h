#ifndef SRC_APP_H_
#define SRC_APP_H_

#define INPUT_FRAME_WIDTH  (960)
#define INPUT_FRAME_HEIGHT (540)

#define FILTER_LENGTH (7)

#define SCALED_FRAME_WIDTH  (INPUT_FRAME_WIDTH / 2)
#define SCALED_FRAME_HEIGHT (INPUT_FRAME_HEIGHT / 2)

#define OUTPUT_FRAME_WIDTH  (SCALED_FRAME_WIDTH - (FILTER_LENGTH - 1))
#define OUTPUT_FRAME_HEIGHT (SCALED_FRAME_HEIGHT - (FILTER_LENGTH - 1))

#define INPUT_FRAME_SIZE (INPUT_FRAME_WIDTH * INPUT_FRAME_HEIGHT)
#define SCALED_FRAME_SIZE (SCALED_FRAME_WIDTH * SCALED_FRAME_HEIGHT)
#define OUTPUT_FRAME_SIZE (OUTPUT_FRAME_WIDTH * OUTPUT_FRAME_HEIGHT)

#define FRAMES (10)

#define STAGES (4)

#define MAX_OUTPUT_SIZE (500 * 1024)

void Scale_SW(const unsigned char * Input, unsigned char * Output);
void Filter_SW(const unsigned char * Input, unsigned char * Output);
void Differentiate_SW(const unsigned char * Input, unsigned char * Output);
int Compress_SW(const unsigned char * Input, unsigned char * Output);

#endif
