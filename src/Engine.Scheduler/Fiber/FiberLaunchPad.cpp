#include "FiberLaunchPad.hpp"

#include <cassert>
#include <Windows.h>
#include <Engine.Common/__macro.hpp>

using namespace hg::engine::scheduler::fiber;
using namespace hg;

void FiberLaunchPad::operator()() {
    DEBUG_ASSERT(self->handle != ::GetFiberData(), "Current fiber and next fiber must never be the same.")
    ::SwitchToFiber(self->handle);
}

void __stdcall FiberLaunchPad::launch(ptr<void> self_) {

    FiberLaunchPad launcher { static_cast<ptr<Fiber>>(self_) };

    while (launcher.self->task) {

        launcher.self->task->delegate();

        // TODO: Refactor, we need this workaround currently, cause delegate is fire and forget
        launcher.self->task = nullptr;
        launcher.self->yield();
    }
}
