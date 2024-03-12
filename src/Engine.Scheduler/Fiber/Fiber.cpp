#include "Fiber.hpp"
#include "FiberPool.hpp"

#ifdef ENV_MSVC
#include <Engine.Common/__macro.hpp>
#include <Windows.h>
#elif defined ENV_GCC
#include <ucontext.h>
#endif

#include <cassert>
#include <Engine.Asserts/Asserts.hpp>

using namespace hg::engine::scheduler::fiber;
using namespace hg;

void Fiber::create(ptr<Fiber> self_, void (*proc_)(void*), void* param_) {

	#ifdef ENV_MSVC
	auto* ctx {
		::CreateFiber(
			default_fiber_stack_size(),
			proc_,
			param_
		)
	};

	#elif defined ENV_GCC
    auto ctx { new ucontext_t {} };
    ctx->uc_stack.ss_sp = malloc(default_fiber_stack_size());
    ctx->uc_stack.ss_size = default_fiber_stack_size();
    ctx->uc_link = 0;
    ::getcontext(ctx);
    ::makecontext(ctx, proc_, 1, param_);

	#endif

	self_->handle = ctx;
	self_->parent = nullptr;
	self_->task = nullptr;
	self_->awaiter.reset();
}

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
	/**
	 *
	 */
	::SwitchToFiber(next);

	#else
    /**
     *
     */
    ::swapcontext(handle, next);

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
    throw {};

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
    throw {};
	#endif
}

constexpr u64 engine::scheduler::fiber::default_fiber_stack_size() noexcept {
	#ifdef ENV_MSVC
	return 4096ui64;
	#else
    return SIGSTKSZ;
	#endif
}
