#include "Park.hpp"

#include "../Process/Schedule.hpp"

using namespace hg::engine::scheduler;
using namespace hg;

#ifdef _DEBUG
worker::PostSwitchHook worker::debugPostSwitchHook = nullptr;
#endif

ref<ParkNode> hg::engine::scheduler::currentParkNode() noexcept {
	if (auto* const f = fiber::self::current()) {
		f->parkNode.fiber = f;
		return f->parkNode;
	}

	// Plain threads never migrate, so a thread_local node stays valid for the thread's lifetime.
	thread_local ParkNode node;
	return node;
}

void hg::engine::scheduler::park(ref<ParkNode> node_) {
	if (auto* const f = fiber::self::current()) {
		f->yield();
		node_.state.store(ParkNode::State::eIdle, std::memory_order::relaxed);
		return;
	}

	while (node_.threadFlag.load(std::memory_order::acquire) != 1) {
		node_.threadFlag.wait(0, std::memory_order::acquire);
	}
	node_.threadFlag.store(0, std::memory_order::relaxed);
	node_.state.store(ParkNode::State::eIdle, std::memory_order::relaxed);
}

void hg::engine::scheduler::unpark(ref<ParkNode> node_) noexcept {
	if (node_.fiber == nullptr) {
		node_.threadFlag.store(1, std::memory_order::release);
		node_.threadFlag.notify_one();
		return;
	}

	const auto prev = node_.state.exchange(ParkNode::State::eWoken, std::memory_order::acq_rel);
	if (prev == ParkNode::State::eParked) {
		node_.fiber->schedule->push(node_.fiber->task);
	}
}
