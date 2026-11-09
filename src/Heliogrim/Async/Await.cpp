#include "Await.hpp"

#include <Engine.Scheduler/Fiber/Fiber.hpp>

void hg::await(const ptr<await_signal_sub_type> signal_) {
	if (engine::scheduler::fiber::self::current() != nullptr) {
		while (!signal_->test(std::memory_order::acquire)) {
			engine::scheduler::fiber::self::yield();
		}
	} else {
		signal_->wait(false, std::memory_order::acquire);
	}
}

void hg::await(ref<Signal> signal_) {
	signal_.wait();
}

void hg::await(ref<JoinCounter> counter_) {
	counter_.wait();
}
