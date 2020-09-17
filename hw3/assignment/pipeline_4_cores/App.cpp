#include "App.h"

#define STAGES (5)

void core_3_process()
{
  // this is a lonely core
  // but you can be nice and give it something
  // to work on :)
}

void core_2_process(int Frame,
                    unsigned char *Input_data,
                    unsigned char **Temp_data)
{
  // if you use core 3, follow this pattern...
  // static unsigned char temp_core_2[FRAME_SIZE];
  // static unsigned char *Input_data_core_2 = temp_core_2;

  // core 2 spins up process on core 3
  // std::thread core_3_thread(&core_3_process);
  // pin_thread_to_cpu(core_3_thread, 3);

  // core 2 does its job
  Scale(Input_data + Frame * FRAME_SIZE, Temp_data[0]);

  // waits for core 3 to finish
  // core_3_thread.join();
}

void core_1_process(int Frame,
                    unsigned char *Input_data,
                    unsigned char **Temp_data)
{
  static unsigned char temp_core_1[FRAME_SIZE];
  static unsigned char *Input_data_core_1 = temp_core_1;
  std::thread core_2_thread;

  if (Frame < FRAMES)
  {
    // core 1 spins up process on core 2
    core_2_thread = std::thread(&core_2_process, Frame,
                                Input_data,
                                Temp_data);
    pin_thread_to_cpu(core_2_thread, 2);
  }

  // core 1 does its job
  if (Frame > 0)
  {
    Filter_horizontal(Input_data_core_1, Temp_data[1]);
  }

  if (Frame < FRAMES)
  {
    // waits for core 2 to finish
    core_2_thread.join();
  }

  unsigned char *Temp = Temp_data[0];
  Temp_data[0] = Input_data_core_1;
  Input_data_core_1 = Temp;
}

void core_0_process(int &Size,
                    int Frame,
                    unsigned char *Input_data,
                    unsigned char **Temp_data,
                    unsigned char *Output_data)
{
  static unsigned char temp_core_0[FRAME_SIZE];
  static unsigned char *Input_data_core_0 = temp_core_0;
  std::thread core_1_thread;
  if (Frame < FRAMES + 1)
  {
    // current core (core 0) spins up process on core 1
    core_1_thread = std::thread(&core_1_process,
                                Frame,
                                Input_data,
                                Temp_data);
    pin_thread_to_cpu(core_1_thread, 1);
  }

  // core 0 does its job
  if (Frame > 1)
  {
    Filter_vertical(Input_data_core_0, Temp_data[2]);
    Differentiate(Temp_data[2], Temp_data[3]);
    Size = Compress(Temp_data[3], Output_data);
  }
  // waits for core 1 to finish
  if (Frame < FRAMES + 1)
  {
    core_1_thread.join();
  }

  unsigned char *Temp = Temp_data[1];
  Temp_data[1] = Input_data_core_0;
  Input_data_core_0 = Temp;
}

int main()
{
  pin_main_thread_to_cpu0();
  unsigned char *Input_data = (unsigned char *)malloc(FRAMES * FRAME_SIZE);
  unsigned char *Output_data = (unsigned char *)malloc(MAX_OUTPUT_SIZE);

  if (Input_data == NULL)
    Exit_with_error();

  if (Output_data == NULL)
    Exit_with_error();

  unsigned char temp_data[STAGES - 1][FRAME_SIZE], *aux[STAGES - 1], **Temp_data;
  Temp_data = (unsigned char **)aux;
  for (int i = 0; i < STAGES - 1; i++)
  {
    aux[i] = (unsigned char *)temp_data + i * FRAME_SIZE;
  }

  Load_data(Input_data);

  stopwatch total_time;
  int Size = 0;
  total_time.start();
  for (int Frame = 0; Frame < FRAMES + 2; Frame++)
  {
    core_0_process(std::ref(Size), Frame, Input_data, Temp_data, Output_data);
  }

  total_time.stop();
  std::cout << "Total time taken by the loop is: " << total_time.latency() << " ns." << std::endl;

  Store_data("Output.bin", Output_data, Size);
  Check_data(Output_data, Size);

  free(Input_data);
  free(Output_data);

  return EXIT_SUCCESS;
}
