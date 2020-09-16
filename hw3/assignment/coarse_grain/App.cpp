#include "App.h"
#include "Stopwatch.h"

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <thread>
#include <vector>

#define FRAME_SIZE (INPUT_WIDTH_SCALE * INPUT_HEIGHT_SCALE)
#define FRAMES (100)
#define STAGES (4)
#define MAX_OUTPUT_SIZE (5000 * 1024)

void Exit_with_error(void)
{
  perror(NULL);
  exit(EXIT_FAILURE);
}

void Load_data(unsigned char *Data)
{
  int Size = FRAMES * FRAME_SIZE;

  FILE *File = fopen("../data/Input.bin", "rb");
  if (File == NULL)
    Exit_with_error();

  if (fread(Data, 1, Size, File) != Size)
    Exit_with_error();

  if (fclose(File) != 0)
    Exit_with_error();
}

// from https://eli.thegreenplace.net/2016/c11-threads-affinity-and-hyperthreading/
void pin_thread_to_cpu(std::thread &t, int cpu_num)
{
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__) || defined(__APPLE__)
  return;
#else
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(cpu_num, &cpuset);
  int rc =
      pthread_setaffinity_np(t.native_handle(), sizeof(cpu_set_t), &cpuset);
  if (rc != 0)
  {
    std::cerr << "Error calling pthread_setaffinity_np: " << rc << "\n";
  }
#endif
}

void Store_data(const char *Filename, unsigned char *Data, int Size)
{
  FILE *File = fopen(Filename, "wb");
  if (File == NULL)
    Exit_with_error();

  if (fwrite(Data, 1, Size, File) != Size)
    Exit_with_error();

  if (fclose(File) != 0)
    Exit_with_error();
}

int Check_data(unsigned char *Data, int Size)
{
  unsigned char *Data_golden = (unsigned char *)malloc(MAX_OUTPUT_SIZE);
  FILE *File = fopen("../data/Golden.bin", "rb");
  if (File == NULL)
    Exit_with_error();

  if (fread(Data_golden, 1, Size, File) != Size)
    Exit_with_error();

  if (fclose(File) != 0)
    Exit_with_error();

  for (int i = 0; i < Size; i++)
  {
    if (Data_golden[i] != Data[i])
    {
      free(Data_golden);
      return i + 1;
    }
  }

  free(Data_golden);
  return 0;
}

int main()
{
  unsigned char *Input_data = (unsigned char *)malloc(FRAMES * FRAME_SIZE);
  unsigned char *Temp_data[STAGES - 1];
  unsigned char *Output_data = (unsigned char *)malloc(MAX_OUTPUT_SIZE);

  if (Input_data == NULL)
    Exit_with_error();

  if (Output_data == NULL)
    Exit_with_error();

  for (int Stage = 0; Stage < STAGES - 1; Stage++)
  {
    Temp_data[Stage] = (unsigned char *)malloc(FRAME_SIZE);
    if (Temp_data[Stage] == NULL)
      Exit_with_error();
  }

  Load_data(Input_data);

  stopwatch total_time;
  int Size = 0;
  total_time.start();
  for (int Frame = 0; Frame < FRAMES; Frame++)
  {
    std::vector<std::thread> ths;
    ths.push_back(std::thread(&Scale, Input_data + Frame * FRAME_SIZE, Temp_data[0], 0, INPUT_HEIGHT_SCALE / 2));
    ths.push_back(std::thread(&Scale, Input_data + Frame * FRAME_SIZE, Temp_data[0], INPUT_HEIGHT_SCALE / 2, INPUT_HEIGHT_SCALE));

    pin_thread_to_cpu(ths[0], 0);
    pin_thread_to_cpu(ths[1], 1);

    for (auto &th : ths)
    {
      th.join();
    }

    Filter(Temp_data[0], Temp_data[1]);
    Differentiate(Temp_data[1], Temp_data[2]);
    Size = Compress(Temp_data[2], Output_data);
  }
  total_time.stop();
  std::cout << "Total time taken by the loop is: " << total_time.latency() << " ns." << std::endl;

  Store_data("Output.bin", Output_data, Size);

  free(Input_data);

  for (int i = 0; i < STAGES - 1; i++)
    free(Temp_data[i]);

  int check_result = Check_data(Output_data, Size);
  if (check_result != 0)
  {
    printf("You got errors in data %d\n", check_result);
  }
  else
  {
    printf("Application completed successfully.\n");
  }

  free(Output_data);
  return EXIT_SUCCESS;
}
