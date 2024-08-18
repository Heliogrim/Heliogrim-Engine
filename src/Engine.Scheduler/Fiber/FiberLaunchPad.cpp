#include "FiberLaunchPad.hpp"

#include <cstddef>
#include <Engine.Asserts/Asserts.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/__macro.hpp>

#if defined(WIN32)
#include <Engine.Common/stdafx.h>
#else
#include <ucontext.h>
#endif

using namespace hg::engine::scheduler::fiber;
using namespace hg;

void FiberLaunchPad::operator()() {

	#ifdef ENV_MSVC
	::hg::assertd(self->handle != ::GetFiberData() /* "Current fiber and next fiber must never be the same." */);
	::SwitchToFiber(self->handle);
	#else
	::hg::assertd(self->handle != self->parent /* "Current fiber and next fiber must never be the same." */);
	::swapcontext(static_cast<ptr<ucontext_t>>(self->parent), static_cast<ptr<ucontext_t>>(self->handle));
	#endif
}

#if defined(WIN32)
void FiberLaunchPad::launch(ptr<void> self_) {
	#else
void FiberLaunchPad::launch(const int ptrLow_, const int ptrHigh_) {

	auto* self_ = reinterpret_cast<ptr<void>>(
		(std::ptrdiff_t { ptrLow_ } & 0x0000'0000'FFFF'FFFF) |
		(std::ptrdiff_t { ptrHigh_ } << 32)
	);

	#endif

	FiberLaunchPad launcher { static_cast<ptr<Fiber>>(self_) };

	while (launcher.self->task) {

		launcher.self->task->delegate();

		// TODO: Refactor, we need this workaround currently, cause delegate is fire and forget
		launcher.self->task = nullptr;
		launcher.self->yield();
	}
}
