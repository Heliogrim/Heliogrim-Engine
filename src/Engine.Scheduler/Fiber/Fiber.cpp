#include "Fiber.hpp"

#include "../Process/BoostCell.hpp"
#include "../Process/Schedule.hpp"
#include "FiberPool.hpp"

#ifdef ENV_MSVC
#include <Engine.Common/stdafx.h>
#include <Engine.Common/__macro.hpp>
#elif defined ENV_GCC
#include <ucontext.h>
#include <signal.h>
#endif

#include <cassert>

import Engine.Asserts;

using namespace hg::engine::scheduler::fiber;
using namespace hg;

#if defined(WIN32)
void Fiber::create(ptr<Fiber> self_, void (*proc_)(void*)) {

	auto* ctx {
		::CreateFiber(
			default_fiber_stack_size(),
			proc_,
			self_
		)
	};

	self_->handle = ctx;
	self_->parent = nullptr;
	self_->task = nullptr;
	self_->schedule = nullptr;
	self_->boost = nullptr;
	self_->cancellation.reset();
}

#else
void Fiber::create(ptr<Fiber> self_, _In_ void (*proc_)(int, int)) {

	auto ctx = new ucontext_t {};
	::getcontext(ctx);

	ctx->uc_stack.ss_sp = malloc(default_fiber_stack_size());
	ctx->uc_stack.ss_size = default_fiber_stack_size();
	ctx->uc_link = 0;// pointer to the context to continue after return -> we manage this ourself...

	int ptrLow = static_cast<int>(reinterpret_cast<std::ptrdiff_t>(self_) & 0x0000'0000'FFFF'FFFF);
	int ptrHigh = static_cast<int>(reinterpret_cast<std::ptrdiff_t>(self_) >> 32);

	using fwd_proc_type = void (*)();
	auto fwd_proc = (fwd_proc_type)(*proc_);

	::makecontext(ctx, fwd_proc, 2, ptrLow, ptrHigh);

	self_->handle = ctx;
	self_->parent = nullptr;
	self_->task = nullptr;
	self_->schedule = nullptr;
	self_->boost = nullptr;
	self_->cancellation.reset();
}

#endif

bool Fiber::destroy() {

	#ifdef ENV_MSVC
	::DeleteFiber(handle);

	#else
	ptr<ucontext_t> ctx { static_cast<ptr<ucontext_t>>(handle) };
	free(ctx->uc_stack.ss_sp);
	delete ctx;

	#endif

	handle = nullptr;
	parent = nullptr;
	task = nullptr;
	schedule = nullptr;
	boost = nullptr;
	cancellation.reset();

	return true;
}

void Fiber::yield() {

	auto* next { parent };

	::hg::assertd(next != nullptr /* "Dynamic Fiber requires parent to not terminate underlying thread." */);
	::hg::assertd(handle != next /* "Current fiber and next fiber must never be the same." */);

	parent = nullptr;

	#ifdef ENV_MSVC
	::SwitchToFiber(next);
	#else
	::swapcontext(static_cast<ptr<ucontext_t>>(handle), static_cast<ptr<ucontext_t>>(next));
	#endif
}

ptr<Fiber> self::current() noexcept {

	#ifdef ENV_MSVC
	return ::IsThreadAFiber() ? static_cast<ptr<Fiber>>(::GetFiberData()) : nullptr;

	#else
	::hg::todo_panic();

	#endif
}

void self::yield() {
	const auto fiber = current();
	::hg::assertd(fiber != nullptr);
	fiber->yield();
}

bool self::cancellationRequested() noexcept {
	const auto fiber = current();
	return fiber != nullptr && fiber->cancellation != nullptr && fiber->cancellation->isCancellationRequested();
}

void self::bindBoost(const ptr<BoostCell> cell_) noexcept {
	const auto fiber = current();
	if (fiber != nullptr) {
		fiber->boost = cell_;
	}
}

ptr<engine::scheduler::BoostCell> self::boostCell() noexcept {
	const auto fiber = current();
	return fiber != nullptr ? fiber->boost : nullptr;
}

engine::scheduler::Lane self::effectiveLane() noexcept {
	const auto fiber = current();
	if (fiber == nullptr || fiber->task == nullptr) {
		return engine::scheduler::Lane::eInteractive;
	}

	const auto own = fiber->task->lane();
	return fiber->boost != nullptr ? higher(own, fiber->boost->lane.load(std::memory_order::seq_cst)) : own;
}

void self::checkpoint() {
	const auto fiber = current();
	if (fiber == nullptr || fiber->task == nullptr || fiber->schedule == nullptr) {
		return;
	}

	const auto lane = effectiveLane();
	// Boosted above the lane it was popped from: yield now, pushBoostable re-queues it in the raised lane (design 7.3).
	if (lane != fiber->task->lane()) {
		fiber->yield();
		return;
	}

	if (lane == engine::scheduler::Lane::eFrame) {
		return;
	}

	const auto elapsed = std::chrono::steady_clock::now() - fiber->sliceStart;
	if (elapsed >= std::chrono::microseconds { fiber->schedule->sliceMicros() }) {
		fiber->yield();
	}
}

u64 engine::scheduler::fiber::default_fiber_stack_size() noexcept {
	#ifdef ENV_MSVC
	return 4096ui64;
	#else
	return static_cast<u64>(SIGSTKSZ);
	#endif
}
