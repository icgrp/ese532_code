#include "App.h"

#define STAGES (5)

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
  std::thread core_1_thread;
  total_time.start();
  for (int Frame = 0; Frame < FRAMES; Frame++)
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
      core_0_process(std::ref(Size), Input_data_core_0,
                     Temp_data,
                     Output_data);
      // Main thread running on core0, so
      // we can just call the functions for core 0 here
      //
      // we could do this with a thread instead as follows:

      // core_0_thread = std::thread(&core_0_process,
      //                             std::ref(Size),
      //                             Input_data_core_0,
      //                             Temp_data,
      //                             Output_data);
      // pin_thread_to_cpu(core_0_thread, 0);
      // core_0_thread.join();
    }

    if (Frame < FRAMES)
    {
      core_1_thread.join();
    }

    unsigned char *Temp = Temp_data[1];
    Temp_data[1] = Input_data_core_0;
    Input_data_core_0 = Temp;
  }

  core_0_process(std::ref(Size), Input_data_core_0,
                 Temp_data,
                 Output_data);
  
  // If we used a thread instead:

  // core_0_thread = std::thread(&core_0_process,
  //                             std::ref(Size),
  //                             Input_data_core_0,
  //                             Temp_data,
  //                             Output_data);
  // pin_thread_to_cpu(core_0_thread, 0);
  // core_0_thread.join();

  total_time.stop();
  std::cout << "Total time taken by the loop is: " << total_time.latency() << " ns." << std::endl;

  Store_data("Output.bin", Output_data, Size);

  free(Input_data);

  for (int i = 0; i < STAGES - 1; i++)
    free(Temp_data[i]);
  free(Input_data_core_0);

  Check_data(Output_data, Size);

  free(Output_data);
  return EXIT_SUCCESS;
}
