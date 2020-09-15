#include "App.h"
#include "Stopwatch.h"

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <thread>
#include <vector>

#define FRAME_SIZE (960 * 540)
#define FRAMES (10)
#define STAGES (4)
#define MAX_OUTPUT_SIZE (500 * 1024)

void Exit_with_error(void)
{
  perror(NULL);
  exit(EXIT_FAILURE);
}

void Load_data(unsigned char * Data)
{
  int Size = FRAMES * FRAME_SIZE;

  FILE * File = fopen("../data/Input.bin", "rb");
  if (File == NULL)
    Exit_with_error();

  if (fread(Data, 1, Size, File) != Size)
    Exit_with_error();

  if (fclose(File) != 0)
    Exit_with_error();
}

void pin_thread_to_cpu(std::thread& t, int cpu_num) {
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(cpu_num, &cpuset);
  int rc =
      pthread_setaffinity_np(t.native_handle(), sizeof(cpu_set_t), &cpuset);
  if (rc != 0) {
    std::cerr << "Error calling pthread_setaffinity_np: " << rc << "\n";
  }
}

void Store_data(const char * Filename, unsigned char * Data, int Size)
{
  FILE * File = fopen(Filename, "wb");
  if (File == NULL)
    Exit_with_error();

  if (fwrite(Data, 1, Size, File) != Size)
    Exit_with_error();

  if (fclose(File) != 0)
    Exit_with_error();
}

int Check_data(unsigned char * Data, int Size)
{
  unsigned char *Data_golden = (unsigned char *)malloc(MAX_OUTPUT_SIZE);
  FILE * File = fopen("../data/Golden.bin", "rb");
  if (File == NULL)
    Exit_with_error();

  if (fread(Data_golden, 1, Size, File) != Size)
    Exit_with_error();

  if (fclose(File) != 0)
    Exit_with_error();

  for(int i=0; i<Size; i++){
    if(Data_golden[i] != Data[i]){
      free(Data_golden);
      return i+1;
    }
  }

  free(Data_golden);
  return 0;
}

int main()
{
  unsigned char * Input_data = (unsigned char *)malloc(FRAMES * FRAME_SIZE);
  unsigned char * Temp_data[STAGES - 1];
  unsigned char * Output_data = (unsigned char *)malloc(MAX_OUTPUT_SIZE);

  for (int Stage = 0; Stage < STAGES - 1; Stage++)
  {
    Temp_data[Stage] = (unsigned char *)malloc(FRAME_SIZE);
    if (Temp_data[Stage] == NULL)
      Exit_with_error();
  }

  Load_data(Input_data);

  stopwatch time_scale;
  stopwatch time_filter;
  stopwatch time_differentiate;
  stopwatch time_compress;
  stopwatch total_time;

  int Size = 0;
  for (int Frame = 0; Frame < FRAMES; Frame++)
  {
    total_time.start();

    std::vector<std::thread> ths;
    ths.push_back(std::thread(&Scale, Input_data + Frame * FRAME_SIZE, Temp_data[0], 0, 135, 0));
    ths.push_back(std::thread(&Scale, Input_data + Frame * FRAME_SIZE, Temp_data[0], 135, 270, 1));
    ths.push_back(std::thread(&Scale, Input_data + Frame * FRAME_SIZE, Temp_data[0], 270, 405, 2));
    ths.push_back(std::thread(&Scale, Input_data + Frame * FRAME_SIZE, Temp_data[0], 405, HEIGHT, 3));
   
    pin_thread_to_cpu(ths[0], 0);
    pin_thread_to_cpu(ths[1], 4);
    pin_thread_to_cpu(ths[2], 8); 
    pin_thread_to_cpu(ths[3], 12);
    
    time_scale.start();
    for (auto& th : ths) {
        th.join();
    }
    time_scale.stop();

    time_filter.start();
    Filter(Temp_data[0], Temp_data[1]);
    time_filter.stop();

    time_differentiate.start();
    Differentiate(Temp_data[1], Temp_data[2]);
    time_differentiate.stop();

    time_compress.start();
    Size = Compress(Temp_data[2], Output_data);
    time_compress.stop();

    total_time.stop();
  }
  std::cout << "Total latency of Scale is: " << time_scale.latency() << " ns." << std::endl;
  std::cout << "Total latency of Filter is: " << time_filter.latency() << " ns." << std::endl;
  std::cout << "Total latency of Differentiate is: " << time_differentiate.latency() << " ns." << std::endl;
  std::cout << "Total latency of Compress is: " << time_compress.latency() << " ns." << std::endl;
  std::cout << "Total time taken by the loop is: " << total_time.latency() << " ns." << std::endl;
  std::cout << "---------------------------------------------------------------" << std::endl;
  std::cout << "Average latency of Scale per loop iteration is: " << time_scale.avg_latency() << " ns." << std::endl;
  std::cout << "Average latency of Filter per loop iteration is: " << time_filter.avg_latency() << " ns." << std::endl;
  std::cout << "Average latency of Differentiate per loop iteration is: " << time_differentiate.avg_latency() << " ns." << std::endl;
  std::cout << "Average latency of Compress per loop iteration is: " << time_compress.avg_latency() << " ns." << std::endl;
  std::cout << "Average latency of each loop iteration is: " << total_time.avg_latency() << " ns." << std::endl;

  Store_data("Output.bin", Output_data, Size);

  free(Input_data);

  for (int i = 0; i < STAGES - 1; i++)
    free(Temp_data[i]);

  int check_result = Check_data(Output_data, Size);
  if( check_result != 0){
	  printf("You got errors in data %d\n", check_result);
  }else{
	  printf("Application completed successfully.\n");
  }

  free(Output_data);
  return EXIT_SUCCESS;
}
