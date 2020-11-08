#ifndef SRC_STOPWATCH_
#define SRC_STOPWATCH_

#include <chrono>

class stopwatch {
public:
	float total_time, calls;
	std::chrono::time_point<std::chrono::high_resolution_clock> start_time,
			end_time;
	stopwatch() :
			total_time(0), calls(0) {
	}
	;

	inline void reset() {
		total_time = 0;
		calls = 0;
	}

	inline void start() {
		start_time = std::chrono::high_resolution_clock::now();
		calls++;
	}
	;

	inline void stop() {
		end_time = std::chrono::high_resolution_clock::now();
		std::chrono::duration<float, std::milli> elapsed = end_time
				- start_time;
		total_time += elapsed.count();
	}
	;

	// return latency in ms
	inline float latency() {
		return total_time;
	}
	;

	// return latency in ms
	inline float avg_latency() {
		return (total_time / calls);
	}
	;
};

#endif
