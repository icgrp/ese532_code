#ifndef SRC_ENCRYPT_H_
#define SRC_ENCRYPT_H_

#include <stdint.h>

#define MAX_SIZE (0x400000)

void Encrypt_HW(const uint32_t * Input, uint32_t Key, uint32_t * Output,
		int Length);

#endif
