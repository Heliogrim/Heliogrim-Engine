#include "Sleep.hpp"

import Engine.Asserts;
import Engine.Scheduler;

void hg::sleep(const u64 milliseconds_) {
	#ifdef _DEBUG

	#pragma message("Using temporary thread sleep in most likly fiber execution context. Please double check possible race conditions.")
	::hg::engine::scheduler::thread::self::sleepFor(milliseconds_);

	#else

	::hg::todo_panic();

	#endif
}
