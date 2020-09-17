#include "App.h"

#define STAGES (5)

void core_1_process(int Frame,
                    unsigned char *Input_data,
                    unsigned char **Temp_data)
{
  Scale(Input_data + Frame * FRAME_SIZE, Temp_data[0]);
  Filter_core_1(Temp_data[0], Temp_data[1]);
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
  if (Frame < FRAMES)
  {

    core_1_thread = std::thread(&core_1_process,
                                Frame,
                                Input_data,
                                Temp_data);
    pin_thread_to_cpu(core_1_thread, 1);
  }

  if (Frame > 0)
  {
    Filter_core_0(Input_data_core_0, Temp_data[2]);
    Differentiate(Temp_data[2], Temp_data[3]);
    Size = Compress(Temp_data[3], Output_data);
  }

  if (Frame < FRAMES)
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
  for (int Frame = 0; Frame < FRAMES + 1; Frame++)
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
