#include "Fiber.hpp"

#include <cstddef>

#include "FiberPool.hpp"

#ifdef ENV_MSVC
#include <Engine.Common/stdafx.h>
#include <Engine.Common/__macro.hpp>
#elif defined ENV_GCC
#include <ucontext.h>
#include <signal.h>
#endif

#include <cassert>
#include <Engine.Asserts/Asserts.hpp>
#include <Engine.Asserts/Todo.hpp>

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
	self_->awaiter.reset();
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
	self_->awaiter.reset();
}

#endif

bool Fiber::destroy() {

	#ifdef ENV_MSVC
	::DeleteFiber(handle);

	#else
    ucontext_t* ctx { static_cast<ucontext_t*>(handle) };
    free(ctx->uc_stack.ss_sp);
    delete ctx;

	#endif

	handle = nullptr;
	parent = nullptr;
	task = nullptr;
	awaiter.reset();

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

void Fiber::await(mref<FiberAwaitable> awaitable_) {

	::hg::assertd(parent != nullptr /* "Dynamic Fiber requires parent to await payload." */);
	::hg::assertd(handle != parent /* "Current fiber and next fiber must never be the same." */);

	/**
	 *
	 */
	awaiter = std::move(awaitable_);

	/**
	 *
	 */
	yield();

	/**
	 *
	 */
	awaiter.reset();
}

void self::yield() {

	#ifdef ENV_MSVC
	/**
	 *
	 */
	#ifdef _DEBUG
	auto* entry = ::GetFiberData();
	::hg::assertd(entry != nullptr);
	static_cast<ptr<Fiber>>(entry)->yield();
	#else
	static_cast<ptr<Fiber>>(::GetFiberData())->yield();
	#endif

	#else
    /**
     *
     */
	::hg::todo_panic();

	#endif
}

void self::await(mref<FiberAwaitable> awaitable_) {

	#ifdef ENV_MSVC
	/**
	 *
	 */
	#ifdef _DEBUG
	auto* entry = ::GetFiberData();
	::hg::assertd(entry != nullptr);
	static_cast<ptr<Fiber>>(entry)->await(std::forward<FiberAwaitable>(awaitable_));

	#else
	static_cast<ptr<Fiber>>(::GetFiberData())->await(std::forward<FiberAwaitable>(awaitable_));

	#endif

	#else
    /**
     *
     */
	::hg::todo_panic();
	#endif
}

constexpr u64 engine::scheduler::fiber::default_fiber_stack_size() noexcept {
	#ifdef ENV_MSVC
	return 4096ui64;
	#else
    return SIGSTKSZ;
	#endif
}
