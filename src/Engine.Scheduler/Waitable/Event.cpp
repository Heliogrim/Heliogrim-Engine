#include "Event.hpp"

#include <Engine.Asserts/Asserts.hpp>

#include "../Fiber/Park.hpp"

using namespace hg::engine::scheduler;
using namespace hg;

bool Event::isSet() const noexcept {
	return _state.load(std::memory_order::acquire) == 1;
}

void Event::set() noexcept {
	::hg::assertd(_state.load(std::memory_order::relaxed) == 0);
	_state.store(1, std::memory_order::release);

	auto* head = _waiters.takeAll();
	while (head != nullptr) {
		// A woken fiber may re-park at once and overwrite `next`.
		auto* const next = head->next;
		unpark(*head);
		head = next;
	}
}

void Event::wait() {
	if (_state.load(std::memory_order::acquire) == 1) {
		return;
	}

	auto& node = currentParkNode();
	node.state.store(ParkNode::State::eArmed, std::memory_order::relaxed);

	const bool pushed = _waiters.locked(
		[&](auto locked_) {
			if (_state.load(std::memory_order::acquire) == 1) {
				return false;
			}
			locked_.push(node);
			return true;
		}
	);

	if (pushed) {
		park(node);
	} else {
		node.state.store(ParkNode::State::eIdle, std::memory_order::relaxed);
	}
}
