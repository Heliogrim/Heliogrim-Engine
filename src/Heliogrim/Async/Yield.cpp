#include "Yield.hpp"

#include <Engine.Scheduler/Fiber/Fiber.hpp>

void hg::yield() {
	::hg::engine::scheduler::fiber::self::yield();
}

void hg::checkpoint() {
	::hg::engine::scheduler::fiber::self::checkpoint();
}
