#include "App.h"
#include "Stopwatch.h"

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <thread>
#include <vector>

#define FRAME_SIZE (INPUT_WIDTH_SCALE * INPUT_HEIGHT_SCALE)
#define FRAMES (100)
#define STAGES (5)
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

void core_0_process(int &Size, unsigned char *Input_data_core_0,
                    unsigned char **Temp_data,
                    unsigned char *Output_data)
{
  Filter_core_0(Input_data_core_0, Temp_data[2]);
  Differentiate(Temp_data[2], Temp_data[3]);
  Size = Compress(Temp_data[3], Output_data);
}

void core_1_process(int Frame,
                    unsigned char *Input_data,
                    unsigned char **Temp_data)
{
  Scale(Input_data + Frame * FRAME_SIZE, Temp_data[0]);
  Filter_core_1(Temp_data[0], Temp_data[1]);
}

int main()
{
  unsigned char *Input_data = (unsigned char *)malloc(FRAMES * FRAME_SIZE);
  unsigned char *Temp_data[STAGES - 1];
  unsigned char *Output_data = (unsigned char *)malloc(MAX_OUTPUT_SIZE);
  unsigned char *Input_data_core_0 = (unsigned char *)malloc(FRAME_SIZE);

  if (Input_data == NULL)
    Exit_with_error();

  if (Output_data == NULL)
    Exit_with_error();

  if (Input_data_core_0 == NULL)
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
  int Frame;
  std::thread core_1_thread;
  std::thread core_0_thread;
  total_time.start();
  for (Frame = 0; Frame <= FRAMES; Frame++)
  {
    if (Frame < FRAMES)
    {

      core_1_thread = std::thread(&core_1_process, Frame,
                                  Input_data,
                                  Temp_data);
      pin_thread_to_cpu(core_1_thread, 1);
    }

    if (Frame > 0)
    {
      core_0_thread = std::thread(&core_0_process,
                                  std::ref(Size),
                                  Input_data_core_0,
                                  Temp_data,
                                  Output_data);
      pin_thread_to_cpu(core_0_thread, 0);
      core_0_thread.join();
    }

    if (Frame < FRAMES)
    {
      core_1_thread.join();
    }

    unsigned char *Temp = Temp_data[1];
    Temp_data[1] = Input_data_core_0;
    Input_data_core_0 = Temp;
  }

  total_time.stop();
  std::cout << "Total time taken by the loop is: " << total_time.latency() << " ns." << std::endl;

  Store_data("Output.bin", Output_data, Size);

  free(Input_data);

  for (int i = 0; i < STAGES - 1; i++)
    free(Temp_data[i]);
  free(Input_data_core_0);

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
