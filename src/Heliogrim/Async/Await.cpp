#include "Await.hpp"

#include <Engine.Scheduler/Fiber/Fiber.hpp>

void hg::await(const ptr<await_signal_sub_type> signal_) {
	::hg::engine::scheduler::fiber::self::await_signal(signal_);
}

void hg::await(ref<Signal> signal_) {
	signal_.wait();
}

void hg::await(ref<JoinCounter> counter_) {
	counter_.wait();
}
