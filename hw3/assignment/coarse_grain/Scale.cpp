#include "App.h"
#include <iostream>
#include <mutex>

//std::mutex iomutex;
void Scale(const unsigned char * Input, unsigned char * Output, int Y_Start_Idx, int Y_End_Idx, int i)
{
  for (int Y = Y_Start_Idx; Y < Y_End_Idx; Y += 2) {
    for (int X = 0; X < WIDTH; X += 2) {
      Output[(Y / 2) * WIDTH / 2 + (X / 2)] = Input[Y * WIDTH + X];
    }
  }
  //std::lock_guard<std::mutex> iolock(iomutex);
  //std::cout << "Thread #" << i << ": on CPU " << sched_getcpu() << std::endl;
}
