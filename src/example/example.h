#ifndef EXAMPLE_EXAMPLE_H_
#define EXAMPLE_EXAMPLE_H_

#include <atomic>


namespace example {

void run_examples();

void simple_map_example(std::atomic_bool& done_flag);

} // namespace example

#endif // EXAMPLE_EXAMPLE_H_
