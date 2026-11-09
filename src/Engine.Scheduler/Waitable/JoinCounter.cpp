#include "JoinCounter.hpp"

#include <Engine.Asserts/Asserts.hpp>

using namespace hg::engine::scheduler;
using namespace hg;

JoinCounter::JoinCounter(const u32 children_) noexcept :
	_pending(children_ + 1) {}

void JoinCounter::add(const u32 n_) noexcept {
	_pending.fetch_add(n_, std::memory_order_relaxed);
}

void JoinCounter::done() noexcept {
	const auto prev = _pending.fetch_sub(1, std::memory_order_acq_rel);
	::hg::assertd(prev != 0);
	if (prev == 1) {
		_done.set();
	}
}

void JoinCounter::wait() {
	::hg::assertd(not _waited);
	_waited = true;
	done();
	_done.wait();
}

bool JoinCounter::isDone() const noexcept {
	return _done.isSet();
}
