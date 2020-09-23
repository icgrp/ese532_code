#include "App.h"

#define STAGES (4)

int main()
{
  unsigned char *Input_data = (unsigned char *)malloc(FRAMES * FRAME_SIZE);
  unsigned char *Temp_data[STAGES - 1];
  unsigned char *Output_data = (unsigned char *)malloc(MAX_OUTPUT_SIZE);

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

    time_scale.start();
    Scale(Input_data + Frame * FRAME_SIZE, Temp_data[0]);
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
  Check_data(Output_data, Size);

  for (int i = 0; i < STAGES - 1; i++)
    free(Temp_data[i]);
  free(Input_data);
  free(Output_data);

  return EXIT_SUCCESS;
}
