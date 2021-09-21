#include <stdio.h>
#include <stdlib.h>

#include <arm_neon.h>
#include "Stopwatch.h"

void add3_NEON(uint8_t* neon_data){
	/* Create the vector with our data. */
	uint8x16_t data;
	uint8x16_t three = vmovq_n_u8(3);
	for (int i = 0; i < 8; i++) {
		data = vld1q_u8(neon_data);
		data = vaddq_u8(data, three);
		vst1q_u8(neon_data, data);
		neon_data += 16;
	}
}

void add3_c(unsigned int *data){
	for(int i=0;i<128;i++){
		data[i] += 3;
	}
}

void print_NEON (uint8_t* data, char* name) {
	printf ("%s:\n", name);
	for (int i = 0; i < 128; i++) {
		printf ("%02d\n", data[i]);
	}
}

void print_C (unsigned int *data, char* name) {
    printf ("%s:\n", name);
    for (int i = 0; i < 128; i++) {
	    printf ("%02d\n", data[i]);
    }
}

int main () {
    /* Create custom arbitrary data. */
    uint8_t neon_data[128];
    unsigned int c_data[128];
    for(int i=0;i<128;i++){
    	neon_data[i] = i;
    	c_data[i] = i;
    }


    /* Call of the add3 NEON version function. */
    stopwatch timer;
    timer.start();
    add3_NEON(neon_data);
    timer.stop();
    printf("NEON add3 takes %fns \n", timer.latency());

    /* Call of the add3 normal C version function. */
    timer.reset();
    timer.start();
    add3_c(c_data);
    timer.stop();
    printf("C add3 takes %fns \n", timer.latency());

    /* Compare the latency and output */
    print_NEON(neon_data, "data_with_neon");
    print_C(c_data, "data_without_neon");

    return EXIT_SUCCESS;
}
