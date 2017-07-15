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

void simple_map_example()
{
	constexpr size_t item_count = 10'000'000;
	constexpr size_t task_count = 3;
	constexpr size_t task_size = item_count / task_count;

	const auto time_start = std::chrono::high_resolution_clock::now();
	std::cout << "[simple_map_example]" << std::endl;
	to_stream(std::cout, "\titem_count", item_count);
	to_stream(std::cout, "\ttask_count", task_count);
	to_stream(std::cout, "\titems per task", task_size);

	std::function<void()> tasks[task_count];
	std::vector<float> sequence(item_count);
	
	for (size_t i = 0; i < task_count; ++i) {
		const size_t offset = i * task_size;
		auto b = sequence.begin() + offset;
		auto e = (i < task_count - 1) ? (b + task_size) : (sequence.end());

		tasks[i] = [b, e, offset] { std::iota(b, e, float(offset)); };
	}

	std::atomic_size_t wait_counter;
	ts::run(tasks, wait_counter);
	ts::wait_for(wait_counter);

	const auto dur = std::chrono::high_resolution_clock::now() - time_start;
	to_stream(std::cout, "\t----time", dur);
}

} // namespace example
