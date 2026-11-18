#include "Lane.hpp"

#include "Fiber/Fiber.hpp"

using namespace hg::engine::scheduler;
using namespace hg;

Lane hg::engine::scheduler::resolveLane(const Lane requested_) noexcept {

	const auto* const running = fiber::self::current();
	if (running == nullptr || running->task == nullptr) {
		// plain thread, affine thread or worker root: nothing to inherit, and only the pipeline creates Frame work
		return requested_ == Lane::eInherit || requested_ == Lane::eFrame ? Lane::eInteractive : requested_;
	}

	const auto effective = running->task->lane();
	return requested_ == Lane::eInherit ? effective : lower(requested_, effective);
}
