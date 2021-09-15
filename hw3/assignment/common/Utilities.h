#ifndef SRC_UTILITIES_
#define SRC_UTILITIES_

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <thread>
#include <vector>
#include "Constants.h"

void Exit_with_error(const char *s);
void Load_data(unsigned char *Data);
void pin_thread_to_cpu(std::thread &t, int cpu_num);
void pin_main_thread_to_cpu0();
void Store_data(const char *Filename, unsigned char *Data, unsigned int Size);
void Check_data(unsigned char *Data, unsigned int Size);

#endif
