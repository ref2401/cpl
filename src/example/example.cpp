#include "example/example.h"

#include <algorithm>
#include <numeric>
#include <vector>
#include "ts/ts.h"


namespace example {

void map_example()
{
	constexpr size_t float_count = 10;
	constexpr size_t tile_count = 3;
	constexpr size_t tile_float_count = float_count / tile_count;

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
		sequence.begin() + offset + tile_float_count,
		sequence.end(),
		float(offset));

	ts::run(tasks);
	// wait
}

} // namespace example
