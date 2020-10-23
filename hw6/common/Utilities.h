#ifndef SRC_UTILITIES_
#define SRC_UTILITIES_

#define CL_HPP_CL_1_2_DEFAULT_BUILD
#define CL_HPP_TARGET_OPENCL_VERSION 120
#define CL_HPP_MINIMUM_OPENCL_VERSION 120
#define CL_HPP_ENABLE_PROGRAM_CONSTRUCTION_FROM_ARRAY_COMPATIBILITY 1
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS

// OCL_CHECK doesn't work if call has templatized function call
#define OCL_CHECK(error, call)                                                 \
  call;                                                                        \
  if (error != CL_SUCCESS) {                                                   \
    printf("%s:%d Error calling " #call ", error code is: %d\n", __FILE__,     \
           __LINE__, error);                                                   \
    exit(EXIT_FAILURE);                                                        \
  }

#include <vector>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <stdio.h>
#include <CL/cl2.hpp>
#include "EventTimer.h"

// When creating a buffer with user pointer (CL_MEM_USE_HOST_PTR), under the
// hood
// User ptr is used if and only if it is properly aligned (page aligned). When
// not
// aligned, runtime has no choice but to create its own host side buffer that
// backs
// user ptr. This in turn implies that all operations that move data to and from
// device incur an extra memcpy to move data to/from runtime's own host buffer
// from/to user pointer. So it is recommended to use this allocator if user wish
// to
// Create Buffer/Memory Object with CL_MEM_USE_HOST_PTR to align user buffer to
// the
// page boundary. It will ensure that user buffer will be used when user create
// Buffer/Mem Object with CL_MEM_USE_HOST_PTR.
template <typename T> struct aligned_allocator {
  using value_type = T;

  aligned_allocator() {}

  aligned_allocator(const aligned_allocator &) {}

  template <typename U> aligned_allocator(const aligned_allocator<U> &) {}

  T *allocate(std::size_t num) {
    void *ptr = nullptr;

#if defined(_WINDOWS)
    {
      ptr = _aligned_malloc(num * sizeof(T), 4096);
      if (ptr == NULL) {
        std::cout << "Failed to allocate memory" << std::endl;
        exit(EXIT_FAILURE);
      }
    }
#else
    {
      if (posix_memalign(&ptr, 4096, num * sizeof(T)))
        throw std::bad_alloc();
    }
#endif
    return reinterpret_cast<T *>(ptr);
  }
  void deallocate(T *p, std::size_t num) {
#if defined(_WINDOWS)
    _aligned_free(p);
#else
    free(p);
#endif
  }
};

std::vector<cl::Device> get_xilinx_devices();
char* read_binary_file(const std::string &xclbin_file_name, unsigned &nb);
void set_callback(cl::Event event, const char *queue_name);

#endif