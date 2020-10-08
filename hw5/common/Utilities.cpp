#include "Utilities.h"

matrix_type * Create_matrix(void)
{
  matrix_type * Matrix = static_cast<matrix_type *>(
      malloc(MATRIX_WIDTH * MATRIX_WIDTH * sizeof(matrix_type)));
  if (Matrix == NULL)
  {
    std::cerr << "Could not allocate matrix." << std::endl;
    exit (EXIT_FAILURE);
  }
  return Matrix;
}

void Destroy_matrix(matrix_type * Matrix)
{
  free(Matrix);
}

void Randomize_matrix(matrix_type * Matrix)
{
  for (int Y = 0; Y < MATRIX_WIDTH; Y++)
    for (int X = 0; X < MATRIX_WIDTH; X++)
      Matrix[Y * MATRIX_WIDTH + X] = rand();
}

bool Compare_matrices(const matrix_type *Matrix_1,
                      const matrix_type *Matrix_2)
{
  bool Equal = true;
  for (int Y = 0; Y < MATRIX_WIDTH; Y++)
    for (int X = 0; X < MATRIX_WIDTH; X++)
      if ((Matrix_1[Y * MATRIX_WIDTH + X] - Matrix_2[Y * MATRIX_WIDTH + X]) / Matrix_1[Y * MATRIX_WIDTH + X] > 0.01 ||
          (Matrix_1[Y * MATRIX_WIDTH + X] - Matrix_2[Y * MATRIX_WIDTH + X]) / Matrix_1[Y * MATRIX_WIDTH + X] < -0.01)
      //if (Matrix_1[Y * MATRIX_WIDTH + X] != Matrix_2[Y * MATRIX_WIDTH + X])
      {
        std::cout << Matrix_1[Y * MATRIX_WIDTH + X] << "!=" << Matrix_2[Y * MATRIX_WIDTH + X] << std::endl;
        Equal = false;
      }
  return Equal;
}

std::vector<cl::Device> get_xilinx_devices() 
{
    size_t i;
    cl_int err;
    std::vector<cl::Platform> platforms;
    err = cl::Platform::get(&platforms);
    cl::Platform platform;
    for (i  = 0 ; i < platforms.size(); i++){
        platform = platforms[i];
        std::string platformName = platform.getInfo<CL_PLATFORM_NAME>(&err);
        if (platformName == "Xilinx"){
            std::cout << "INFO: Found Xilinx Platform" << std::endl;
            break;
        }
    }
    if (i == platforms.size()) {
        std::cout << "ERROR: Failed to find Xilinx platform" << std::endl;
        exit(EXIT_FAILURE);
    }
   
    //Getting ACCELERATOR Devices and selecting 1st such device 
    std::vector<cl::Device> devices;
    err = platform.getDevices(CL_DEVICE_TYPE_ACCELERATOR, &devices);
    return devices;
}
   
char* read_binary_file(const std::string &xclbin_file_name, unsigned &nb) 
{
    if(access(xclbin_file_name.c_str(), R_OK) != 0) {
        printf("ERROR: %s xclbin not available please build\n", xclbin_file_name.c_str());
        exit(EXIT_FAILURE);
    }
    //Loading XCL Bin into char buffer 
    std::cout << "INFO: Loading '" << xclbin_file_name << "'\n";
    std::ifstream bin_file(xclbin_file_name.c_str(), std::ifstream::binary);
    bin_file.seekg (0, bin_file.end);
    nb = bin_file.tellg();
    bin_file.seekg (0, bin_file.beg);
    char *buf = new char [nb];
    bin_file.read(buf, nb);
    return buf;
}

void multiply_gold(const matrix_type Input_1[MATRIX_WIDTH * MATRIX_WIDTH],
                 const matrix_type Input_2[MATRIX_WIDTH * MATRIX_WIDTH],
		         matrix_type Output[MATRIX_WIDTH * MATRIX_WIDTH])
{
  for (int i = 0; i < MATRIX_WIDTH; i++)
    for (int j = 0; j < MATRIX_WIDTH; j++)
    {
      matrix_type Result = 0;
      for (int k = 0; k < MATRIX_WIDTH; k++)
        Result += Input_1[i * MATRIX_WIDTH + k] * Input_2[k * MATRIX_WIDTH + j];
      Output[i * MATRIX_WIDTH + j] = Result;
    }
}