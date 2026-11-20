#include "Yield.hpp"

import Engine.Scheduler;

void hg::yield() {
	::hg::engine::scheduler::fiber::self::yield();
}

void hg::checkpoint() {
	::hg::engine::scheduler::fiber::self::checkpoint();
}
