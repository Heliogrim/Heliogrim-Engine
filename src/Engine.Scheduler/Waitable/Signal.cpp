#include "Signal.hpp"

#include <Engine.Asserts/Asserts.hpp>

#include "../Fiber/Park.hpp"

using namespace hg::engine::scheduler;
using namespace hg;

bool Signal::isSet() const noexcept {
	return _state.load(std::memory_order::acquire) == 1;
}

void Signal::set() noexcept {
	::hg::assertd(_state.load(std::memory_order::relaxed) == 0);

	ptr<ParkNode> head = _waiters.locked(
		[&](auto locked_) {
			_closed = true;
			return locked_.takeAll();
		}
	);
	// Last access to *this; a waiter may destroy the signal right after.
	_state.store(1, std::memory_order::release);

	while (head != nullptr) {
		// A woken fiber may re-park at once and overwrite `next`.
		auto* const next = head->next;
		unpark(*head);
		head = next;
	}
}

void Signal::wait() {
	if (_state.load(std::memory_order::acquire) == 1) {
		return;
	}

	auto& node = currentParkNode();
	node.state.store(ParkNode::State::eArmed, std::memory_order::relaxed);

	const bool pushed = _waiters.locked(
		[&](auto locked_) {
			if (_closed) {
				return false;
			}
			locked_.push(node);
			return true;
		}
	);

	if (pushed) {
		park(node);
	} else {
		while (_state.load(std::memory_order::acquire) != 1) {
			// the setter's store is imminent
		}
		node.state.store(ParkNode::State::eIdle, std::memory_order::relaxed);
	}
}

void Signal::reset() noexcept {
	_waiters.locked(
		[&](auto locked_) {
			::hg::assertd(locked_.empty());
			_closed = false;
		}
	);
	_state.store(0, std::memory_order::relaxed);
}
