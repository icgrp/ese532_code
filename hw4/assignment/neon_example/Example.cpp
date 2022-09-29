#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <arm_neon.h>
#include "Stopwatch.h"

#define LOOP_SIZE 1024

void add3_NEON(uint8_t* neon_data){
	/* Create the vector with our data. */
	uint8x16_t data;
	uint8x16_t three = vmovq_n_u8(3);
	for (int i = 0; i < LOOP_SIZE/16; i++) {
		data = vld1q_u8(neon_data);
		data = vaddq_u8(data, three);
		vst1q_u8(neon_data, data);
		neon_data += 16;
	}
}

void add3_c(unsigned int *data){
	for(int i=0;i<LOOP_SIZE;i++){
		data[i] += 3;
	}
}

void print_NEON (uint8_t* data, char* name) {
	printf ("%s:\n", name);
	for (int i = 0; i < LOOP_SIZE; i++) {
		printf ("%02d\n", data[i]);
	}
}

void print_C (unsigned int *data, char* name) {
    printf ("%s:\n", name);
    for (int i = 0; i < LOOP_SIZE; i++) {
	    printf ("%02d\n", data[i]);
    }
}

bool compare_results (unsigned int *c_data, uint8_t *neon_data){
    for (int i = 0; i < LOOP_SIZE; i++){
        if((uint8_t)c_data[i] != neon_data[i]){
            return false;
        }
    }
    return true;
}

int main () {
    /* Create custom arbitrary data. */
    uint8_t neon_data[LOOP_SIZE];
    unsigned int c_data[LOOP_SIZE];
    for(int i=0;i<LOOP_SIZE;i++){
    	neon_data[i] = i%128;
    	c_data[i] = i%128;
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
    //print_NEON(neon_data, "data_with_neon");
    //print_C(c_data, "data_without_neon");
    bool equal = compare_results(c_data, neon_data);
    std::cout << "Results are " << (equal ? "equal" : "NOT equal") << std::endl;

    return EXIT_SUCCESS;
}
