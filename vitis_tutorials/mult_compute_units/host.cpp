/**********
 Copyright (c) 2020, Xilinx, Inc.
 All rights reserved.

 Redistribution and use in source and binary forms, with or without modification,
 are permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice,
 this list of conditions and the following disclaimer.

 2. Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.

 3. Neither the name of the copyright holder nor the names of its contributors
 may be used to endorse or promote products derived from this software
 without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 ARE DISCLAIMED.
 IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 INDIRECT,
 INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 LIMITED TO,
 PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 THIS SOFTWARE,
 EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 **********/

/*This is simple Example of Multiple Compute units to showcase how a single
 kernel
 can be instantiated into Multiple compute units. Host code will show how to use
 multiple compute units and run them concurrently. */
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <vector>

// OpenCL utility layer include
#include "xcl2.hpp"

auto constexpr data_hw = 1024 * 1024 * 4;
auto constexpr num_cu = 4;

//////////////MAIN FUNCTION//////////////
int main(int argc, char **argv) {
	if (argc != 2) {
		std::cout << "Usage: " << argv[0] << "<XCLBIN File>" << std::endl;
		return EXIT_FAILURE;
	}
	std::string binaryFile = argv[1];
	int data_size = data_hw;
	cl_int err;
	if (xcl::is_emulation()) {
		data_size = 4096 * 4;
	}
	cl::CommandQueue q;
	cl::Context context;
	std::vector<cl::Kernel> krnls(num_cu);

	// I/O data vectors
	int *source_in1[num_cu];
	int *source_in2[num_cu];
	int *source_hw_results[num_cu];

	// get_xil_devices() is a utility API which will find the xilinx
	// platforms and will return list of devices connected to Xilinx platform
	auto devices = xcl::get_xil_devices();

	// read_binary_file() is a utility API which will load the binaryFile
	// and will return the pointer to file buffer.
	auto fileBuf = xcl::read_binary_file(binaryFile);
	cl::Program::Binaries bins { { fileBuf.data(), fileBuf.size() } };
	bool valid_device = false;
	for (unsigned int i = 0; i < devices.size(); i++) {
		auto device = devices[i];
		// Creating Context and Command Queue for selected Device
		OCL_CHECK(err, context = cl::Context(device, NULL, NULL, NULL, &err));
		OCL_CHECK(err,
				q = cl::CommandQueue(context, device,
						CL_QUEUE_PROFILING_ENABLE
								| CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, &err));

		std::cout << "Trying to program device[" << i << "]: "
				<< device.getInfo<CL_DEVICE_NAME>() << std::endl;
		cl::Program program(context, { device }, bins, NULL, &err);
		if (err != CL_SUCCESS) {
			std::cout << "Failed to program device[" << i
					<< "] with xclbin file!\n";
		} else {
			std::cout << "Device[" << i << "]: program successful!\n";
			// Creating Kernel objects
			for (int i = 0; i < num_cu; i++) {
				OCL_CHECK(err, krnls[i] = cl::Kernel(program, "vadd", &err));
			}
			valid_device = true;
			break; // we break because we found a valid device
		}
	}
	if (!valid_device) {
		std::cout << "Failed to program any device found, exit!\n";
		exit(EXIT_FAILURE);
	}

	// Creating buffers
	auto chunk_size = data_size / num_cu;
	size_t vector_size_bytes = sizeof(int) * chunk_size;
	std::vector<cl::Buffer> buffer_in1(num_cu);
	std::vector<cl::Buffer> buffer_in2(num_cu);
	std::vector<cl::Buffer> buffer_output(num_cu);

	for (int i = 0; i < num_cu; i++) {
		OCL_CHECK(err,
				buffer_in1[i] = cl::Buffer( context, CL_MEM_READ_ONLY, vector_size_bytes, NULL, &err));
		OCL_CHECK(err,
				buffer_in2[i] = cl::Buffer( context, CL_MEM_READ_ONLY, vector_size_bytes, NULL, &err));
		OCL_CHECK(err,
				buffer_output[i] = cl::Buffer( context, CL_MEM_WRITE_ONLY, vector_size_bytes, NULL, &err));
		source_in1[i] = (int *) q.enqueueMapBuffer(buffer_in1[i], CL_TRUE,
				CL_MAP_WRITE, 0, vector_size_bytes);
		source_in2[i] = (int *) q.enqueueMapBuffer(buffer_in2[i], CL_TRUE,
				CL_MAP_WRITE, 0, vector_size_bytes);
		source_hw_results[i] = (int *) q.enqueueMapBuffer(buffer_output[i],
				CL_TRUE, CL_MAP_READ, 0, vector_size_bytes);
	}

	for (int i = 0; i < num_cu; i++) {
		int narg = 0;

		// Setting kernel arguments
		OCL_CHECK(err, err = krnls[i].setArg(narg++, buffer_in1[i]));
		OCL_CHECK(err, err = krnls[i].setArg(narg++, buffer_in2[i]));
		OCL_CHECK(err, err = krnls[i].setArg(narg++, buffer_output[i]));
		OCL_CHECK(err, err = krnls[i].setArg(narg++, chunk_size));

		// Copy input data to device global memory
		OCL_CHECK(err, err = q.enqueueMigrateMemObjects( { buffer_in1[i],
				buffer_in2[i] }, 0 /* 0 means from host*/));

		// Launch the kernel
		OCL_CHECK(err, err = q.enqueueTask(krnls[i]));
	}

	OCL_CHECK(err, err = q.finish());

	// Copy result from device global memory to host local memory
	for (int i = 0; i < num_cu; i++) {
		OCL_CHECK(err,
				err = q.enqueueMigrateMemObjects( { buffer_output[i] },
						CL_MIGRATE_MEM_OBJECT_HOST));
	}

	for (int i = 0; i < num_cu; i++) {
		q.enqueueUnmapMemObject(buffer_in1[i], source_in1[i]);
		q.enqueueUnmapMemObject(buffer_in2[i], source_in2[i]);
		q.enqueueUnmapMemObject(buffer_output[i], source_hw_results[i]);
	}
	OCL_CHECK(err, err = q.finish());

}
