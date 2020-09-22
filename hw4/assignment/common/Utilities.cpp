#include "Utilities.h"

void Exit_with_error(void)
{
    perror(NULL);
    exit(EXIT_FAILURE);
}

void Load_data(unsigned char *Data)
{
    unsigned int Size = FRAMES * FRAME_SIZE;

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

void pin_main_thread_to_cpu0()
{
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__) || defined(__APPLE__)
    return;
#else
    pthread_t thread;
    thread = pthread_self();
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(0, &cpuset);
    int rc =
        pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset);
    if (rc != 0)
    {
        std::cerr << "Error calling pthread_setaffinity_np: " << rc << "\n";
    }
#endif
}

void Store_data(const char *Filename, unsigned char *Data, unsigned int Size)
{
    FILE *File = fopen(Filename, "wb");
    if (File == NULL)
        Exit_with_error();

    if (fwrite(Data, 1, Size, File) != Size)
        Exit_with_error();

    if (fclose(File) != 0)
        Exit_with_error();
}

void Check_data(unsigned char *Data, unsigned int Size)
{
    int error_code;
    unsigned char *Data_golden = (unsigned char *)malloc(MAX_OUTPUT_SIZE);
    FILE *File = fopen("../data/Golden.bin", "rb");
    if (File == NULL)
        Exit_with_error();

    if (fread(Data_golden, 1, Size, File) != Size)
        Exit_with_error();

    if (fclose(File) != 0)
        Exit_with_error();

    for (unsigned int i = 0; i < Size; i++)
    {
        if (Data_golden[i] != Data[i])
        {
            free(Data_golden);
            error_code = i + 1;
        }
    }

    free(Data_golden);
    error_code = 0;

    if (error_code != 0)
    {
        printf("You got errors in data %d\n", error_code);
    }
    else
    {
        printf("Application completed successfully.\n");
    }
}