#include "example/example.h"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <numeric>
#include <vector>
#include "ts/task_system.h"

namespace {

template<typename T>
inline void to_stream(std::ostream& o, const char* title, const T& v)
{
	if (title)
		o << title << ": ";
		
	o << v << ';' << std::endl;
}

inline void to_stream(std::ostream& o, const char* title, const std::chrono::high_resolution_clock::duration& dur)
{
	using dur_milli_t = std::chrono::duration<float, std::milli>;
	using dur_micro_t = std::chrono::duration<float, std::micro>;

	o << title << ": " 
		<< std::chrono::duration_cast<dur_milli_t>(dur).count() << " ms; ("
		<< std::chrono::duration_cast<dur_micro_t>(dur).count() << " mc);" 
		<< std::endl;
}

} // namespace


namespace example {

void run_examples()
{
	std::atomic_bool done_flag = false;

	ts::task_desc task(simple_map_example, std::ref(done_flag));
	ts::run(&task, 1);

	while (!done_flag)
		; // spin-wait
}

void simple_map_example(std::atomic_bool& done_flag)
{
	constexpr size_t float_count = 10'000'000;
	constexpr size_t tile_count = 3;
	constexpr size_t tile_float_count = float_count / tile_count;

	auto time_start = std::chrono::high_resolution_clock::now();
	std::cout << "[simple_map_example]" << std::endl;
	to_stream(std::cout, "\tfloat_count", float_count);
	to_stream(std::cout, "\ttile_count", tile_count);
	to_stream(std::cout, "\tfloats per tile", tile_float_count);

	ts::task_desc tasks[tile_count];
	std::vector<float> sequence(float_count);
	
	using it = decltype(sequence)::iterator;

	for (size_t i = 0; i < tile_count - 1; ++i) {
		const size_t offset = i * tile_float_count;
		it b = sequence.begin() + offset;
		it e = b + tile_float_count;
		tasks[i] = ts::task_desc(std::iota<it, float>, b, e, float(offset));
	}

	const size_t offset = (tile_count - 1) * tile_float_count;
	tasks[tile_count - 1] = ts::task_desc(std::iota<it, float>,
		sequence.begin() + offset,
		sequence.end(),
		float(offset));

	std::atomic_size_t wait_counter;
	ts::run(tasks, &wait_counter);
	ts::wait_for(&wait_counter);

	auto dur = std::chrono::high_resolution_clock::now() - time_start;
	to_stream(std::cout, "\t----time", dur);
	done_flag = true;
}

} // namespace example
