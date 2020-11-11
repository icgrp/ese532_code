#include<chrono>
#include<ctime>
#include<cstdio>
#include<cstdlib>
#include<cmath>
#include<iostream>
#include<vector>
#include<utility>
#include<random>
#include"xcl2.hpp"
#include"sizes.h"
#include"common.h"

//#define DEBUGX 
using namespace std;
using namespace std::chrono;

unsigned int* input_doc_words;
vector<unsigned long> profile_weights;
unsigned int* bloom_filter;
vector<unsigned int> starting_doc_id;
vector<unsigned long> fpga_profileScore;
vector<unsigned int> doc_sizes;
vector<unsigned long> cpu_profileScore;

default_random_engine generator;
normal_distribution<double> distribution(3500, 500);

unsigned int total_num_docs;
unsigned size = 0;
unsigned block_size;

string kernel_name = "runOnfpga";
const char* kernel_name_charptr = kernel_name.c_str();
unsigned int bloom_filter_size = 1L << bloom_size;
unsigned int profile_size = 1L << 24;
unsigned size_per_iter_const = 512 * 1024;
unsigned size_per_iter;

unsigned doc_len() {
	unsigned int len = distribution(generator);
	if (len < 100) {
		len = 100;
	}
//    if(len > 4000) { len = 3500;}
	return len;
}

void setupData() {
	starting_doc_id.reserve(total_num_docs);
	fpga_profileScore.reserve(total_num_docs);
	cpu_profileScore.reserve(total_num_docs);

	//  h_docInfo.reserve( total_num_docs );

	doc_sizes.reserve(total_num_docs);
	unsigned unpadded_size = 0;

	for (unsigned i = 0; i < total_num_docs; i++) {
		unsigned len_doc = doc_len();
		starting_doc_id[i] = unpadded_size;
		unpadded_size += len_doc;
		doc_sizes[i] = len_doc;
	}

	size = unpadded_size & (~(block_size - 1));
	if (unpadded_size & (block_size - 1))
		size += block_size;

	// double mbytes = size*sizeof(int)/(1000000.0);

	printf("Creating documents - total size : %.3f MBytes (%d words)\n",
			size * sizeof(int) / 1000000.0, size);
	// std::cout << "Creating documents of total size = "<< size  << " words" << endl;

	profile_weights.reserve((1L << 24));
	std::cout << "Creating profile weights" << endl;
	std::cout << endl;

	for (unsigned i = 0; i < (1L << 24); i++) {
		profile_weights[i] = 0;
	}
}

int main(int argc, char** argv) {
	int num_iter;

	switch (argc) {
	case 2:
		total_num_docs = atoi(argv[1]);
		num_iter = 2;
		break;
	case 3:
		total_num_docs = atoi(argv[1]);
		num_iter = atoi(argv[2]);
		break;
	default:
		cout << "Incorrect number of arguments" << endl;
		return 0;
	}

	std::cout << "Initializing data" << endl;
	block_size = num_iter * 64;
	setupData();

	//printf ("Sending data on FPGA  Doc_sizes = %lu\n ", doc_sizes.size());
	//std::cout << "Sending data on FPGA  Doc_sizes " <<  doc_sizes.size() << endl;
	//std::cout << "Input Doc Words " <<  input_doc_words.size() << endl;
	if ((size / num_iter) % 64 != 0) {
		printf(
				"--------------------------------------------------------------------\n");
		printf(
				"ERROR: The number of word per iterations must be a multiple of 64\n");
		printf(
				"       Total words = %d, Number of iterations = %d, Word per iterations = %d\n",
				size, num_iter, size / num_iter);
		printf("       Skipping FPGA kernel execution\n");
		exit(-1);
	}

	// Boilerplate code to load the FPGA binary, create the kernel and command queue
	vector<cl::Device> devices = xcl::get_xil_devices();
	cl::Device device = devices[0];
	cl::Context context(device);
	cl::CommandQueue q(context, device,
			CL_QUEUE_PROFILING_ENABLE | CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE);

	string run_type =
			xcl::is_emulation() ?
					(xcl::is_hw_emulation() ? "hw_emu" : "sw_emu") : "hw";
	string binary_file = kernel_name + "_" + run_type + ".xclbin";
	cl::Program::Binaries bins = xcl::import_binary_file(binary_file);
	cl::Program program(context, devices, bins);
	cl::Kernel kernel(program, kernel_name_charptr, NULL);

	unsigned int total_size = size;
	bool load_filter = true;

	// Create buffers
	cl::Buffer buffer_bloom_filter(context, CL_MEM_READ_ONLY,
			bloom_filter_size * sizeof(uint));
	cl::Buffer buffer_input_doc_words(context, CL_MEM_READ_ONLY,
			total_size * sizeof(uint));
	cl::Buffer buffer_output_inh_flags(context, CL_MEM_WRITE_ONLY,
			total_size * sizeof(char));

	bloom_filter = (unsigned int *) q.enqueueMapBuffer(buffer_bloom_filter,
			CL_TRUE, CL_MAP_WRITE, 0, bloom_filter_size * sizeof(uint));
	input_doc_words = (unsigned int *) q.enqueueMapBuffer(
			buffer_input_doc_words, CL_TRUE, CL_MAP_WRITE, 0,
			total_size * sizeof(uint));
	unsigned char* output_inh_flags = (unsigned char *) q.enqueueMapBuffer(
			buffer_output_inh_flags, CL_TRUE, CL_MAP_READ, 0,
			total_size * sizeof(char));

	for (unsigned i = 0; i < size; i++) {
		input_doc_words[i] = docTag;
	}
	for (unsigned doci = 0; doci < total_num_docs; doci++) {
		unsigned start_dimm1 = starting_doc_id[doci];
		unsigned size_1 = doc_sizes[doci];
		unsigned term;
		unsigned freq;

		//printf ("size_1 %d\n", size_1);
		for (unsigned i = 0; i < size_1; i++) {
			term = (rand() % ((1L << 24) - 1));   // 24 Bit
			freq = (rand() % 254) + 1;            //  8 Bit
			//printf("TERM = %x, Freq=%x\n", term , freq);
			input_doc_words[start_dimm1 + i] = (term << 8) | freq;
		}
	}

	for (unsigned i = 0; i < (1L << bloom_size); i++) {
		bloom_filter[i] = 0x0;
	}

	for (unsigned i = 0; i < 16384; i++) {    // Bloom filter size 1L<<14 =16384
		unsigned entry = (rand() % (1 << 24));

		profile_weights[entry] = 10;
		unsigned hash_pu = MurmurHash2(&entry, 3, 1);
		unsigned hash_lu = MurmurHash2(&entry, 3, 5);

		unsigned hash1 = hash_pu & hash_bloom;
		unsigned hash2 = (hash_pu + hash_lu) & hash_bloom;

		bloom_filter[hash1 >> 5] |= 1 << (hash1 & 0x1f);
		bloom_filter[hash2 >> 5] |= 1 << (hash2 & 0x1f);
	}

	// Set buffer kernel arguments (needed to migrate the buffers in the correct memory)
	kernel.setArg(0, buffer_output_inh_flags);
	kernel.setArg(1, buffer_input_doc_words);
	kernel.setArg(2, buffer_bloom_filter);

	// Make buffers resident in the device
	q.enqueueMigrateMemObjects( { buffer_bloom_filter, buffer_input_doc_words,
			buffer_output_inh_flags }, CL_MIGRATE_MEM_OBJECT_CONTENT_UNDEFINED);

	// Specify size of sub-buffers for each iteration
	unsigned subbuf_doc_sz = size / num_iter;
	unsigned subbuf_inh_sz = size / num_iter;

	// Declare sub-buffer regions which specify offset and size for each iteration
	cl_buffer_region subbuf_inh_info[num_iter];
	cl_buffer_region subbuf_doc_info[num_iter];

	// Declare sub-buffers for each iteration
	cl::Buffer subbuf_inh_flags[num_iter];
	cl::Buffer subbuf_doc_words[num_iter];

	// Define sub-buffers from buffers based on sub-buffer regions
	for (int i = 0; i < num_iter; i++) {
		subbuf_inh_info[i]= {i*subbuf_inh_sz*sizeof(char), subbuf_inh_sz*sizeof(char)};
		subbuf_doc_info[i]= {i*subbuf_doc_sz*sizeof(uint), subbuf_doc_sz*sizeof(uint)};
		subbuf_inh_flags[i] = buffer_output_inh_flags.createSubBuffer(CL_MEM_WRITE_ONLY, CL_BUFFER_CREATE_TYPE_REGION, &subbuf_inh_info[i]);
		subbuf_doc_words[i] = buffer_input_doc_words.createSubBuffer (CL_MEM_READ_ONLY, CL_BUFFER_CREATE_TYPE_REGION, &subbuf_doc_info[i]);
	}

	printf("\n");
	double mbytes_total = (double) (size * sizeof(int))
			/ (double) (1000 * 1000);
	double mbytes_block = mbytes_total / num_iter;
	printf(" Processing %.3f MBytes of data\n", mbytes_total);
	if (num_iter > 1) {
		printf(
				" Splitting data in %d sub-buffers of %.3f MBytes for FPGA processing\n",
				num_iter, mbytes_block);
	}

	// Create Events to co-ordinate read,compute and write for each iteration
	vector<cl::Event> wordWait;
	vector<cl::Event> flagWait;

	printf(
			"--------------------------------------------------------------------\n");

	chrono::high_resolution_clock::time_point t1, t2;
	t1 = chrono::high_resolution_clock::now();

	// Set Kernel arguments and load bloom filter coefficients
	cl::Event buffDone, krnlDone;

	total_size = 0;
	load_filter = true;
	kernel.setArg(3, total_size);
	kernel.setArg(4, load_filter);
	q.enqueueMigrateMemObjects( { buffer_bloom_filter }, 0, NULL, &buffDone);
	wordWait.push_back(buffDone);
	q.enqueueTask(kernel, &wordWait, &krnlDone);

	// Set Kernel arguments. Read,enqueue the kernel and write for each iteration
	for (int i = 0; i < num_iter; i++) {
		vector<cl::Event> krnlWait;
		cl::Event flagDone;

		cl::Event buffDone, krnlDone;
		total_size = subbuf_doc_info[i].size / sizeof(uint);
		load_filter = false;
		kernel.setArg(0, subbuf_inh_flags[i]);
		kernel.setArg(1, subbuf_doc_words[i]);
		kernel.setArg(3, total_size);
		kernel.setArg(4, load_filter);
		q.enqueueMigrateMemObjects( { subbuf_doc_words[i] }, 0, &wordWait,
				&buffDone);
		wordWait.pop_back();
		wordWait.push_back(buffDone);
		q.enqueueTask(kernel, &wordWait, &krnlDone);
		krnlWait.push_back(krnlDone);

		q.enqueueMigrateMemObjects( { subbuf_inh_flags[i] },
				CL_MIGRATE_MEM_OBJECT_HOST, &krnlWait, &flagDone);
		set_callback(flagDone, "oooqueue");
		flagWait.push_back(flagDone);
	}

	// Create variables to keep track of number of words needed by CPU to compute score and number of words processed by FPGA such that CPU processing can overlap with FPGA
	unsigned int curr_entry;
	unsigned char inh_flags;
	unsigned int available = 0;
	unsigned int needed = 0;
	unsigned int iter = 0;

	for (unsigned int doc = 0, n = 0; doc < total_num_docs; doc++) {
		unsigned long ans = 0;
		unsigned int size = doc_sizes[doc];

		// Calculate size by needed by CPU for processing next document score
		needed += size;
		if (needed > available) {
			clWaitForEvents(1, (const cl_event *) &flagWait[iter]);

			std::cout << "waiting..." << std::endl;
			available += subbuf_doc_info[iter].size / sizeof(uint);
			iter++;
		}

		// Check if flgas processed by FPGA is greater than needed by CPU. Else, block CPU
		// Update the number of available words and sub-buffer count(iter)
		for (unsigned i = 0; i < size; i++, n++) {
			curr_entry = input_doc_words[n];
			inh_flags = output_inh_flags[n];

			if (inh_flags) {
				unsigned frequency = curr_entry & 0x00ff;
				unsigned word_id = curr_entry >> 8;

				ans += profile_weights[word_id] * (unsigned long) frequency;
			}
		}
		fpga_profileScore[doc] = ans;
	}

	t2 = chrono::high_resolution_clock::now();
	chrono::duration<double> perf_all_sec = chrono::duration_cast<
			duration<double>>(t2 - t1);

	cl_ulong f1 = 0;
	cl_ulong f2 = 0;
	wordWait.front().getProfilingInfo(CL_PROFILING_COMMAND_QUEUED, &f1);
	flagWait.back().getProfilingInfo(CL_PROFILING_COMMAND_END, &f2);
	double perf_hw_ms = (f2 - f1) / 1000000.0;

	printf(" Executed FPGA accelerated version  | %10.4f ms   ( FPGA %.3f ms )",
			1000 * perf_all_sec.count(), perf_hw_ms);
	printf("\n");

	runOnCPU(doc_sizes.data(), input_doc_words, bloom_filter,
			profile_weights.data(), cpu_profileScore.data(), total_num_docs,
			size);

	printf(
			"--------------------------------------------------------------------\n");

	for (unsigned doci = 0; doci < total_num_docs; doci++) {
		if (cpu_profileScore[doci] != fpga_profileScore[doci]) {
			std::cout << " Verification: FAILED " << endl << " : doc[" << doci
					<< "]" << " score: CPU = " << cpu_profileScore[doci]
					<< ", FPGA = " << fpga_profileScore[doci] << endl;
			return 0;
		}
	}
	cout << " Verification: PASS" << endl;
	cout << endl;
	q.enqueueUnmapMemObject(buffer_bloom_filter, bloom_filter);
	q.enqueueUnmapMemObject(buffer_input_doc_words, input_doc_words);
	q.enqueueUnmapMemObject(buffer_output_inh_flags, output_inh_flags);
	q.finish();

	return 0;
}

