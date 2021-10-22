#pragma once

#ifdef _DEBUG
#include <cassert>
#endif

#include <Windows.h>

#include "Fiber.hpp"

namespace ember::engine::scheduler::fiber {

    struct FiberLaunchPad {
    public:
        ptr<Fiber> self;

        void assign(_In_ Fiber::handle_type parent_, _In_ ptr<Fiber> self_, _In_ task::__TaskDelegate task_) {
            self = self_;
            self->parent = parent_;

            self->task = task_;
            // TODO: Refactor
            const_cast<ptr<task::TaskDelegate>>(task_)->fiber() = self;
        }

    private:
        void unassign() {
            // TODO: Refactor
            if (self->task != nullptr) {
                const_cast<ptr<task::TaskDelegate>>(self->task)->fiber() = nullptr;
            }

            self->task = nullptr;
        }

    public:
        void operator()() {
            DEBUG_ASSERT(self->handle != ::GetFiberData(), "Current fiber and next fiber must never be the same.")
            ::SwitchToFiber(self->handle);
        }

        static void __stdcall launch(_In_ ptr<void> self_) {

            FiberLaunchPad launcher { static_cast<ptr<Fiber>>(self_) };

            while (launcher.self->task) {

                launcher.self->task->delegate();
                // TODO: Refactor, we need this workaround currently, cause delegate is fire and forget
                launcher.self->task = nullptr;
                launcher.unassign();

                launcher.self->yield();
            }
        }
    };
}
