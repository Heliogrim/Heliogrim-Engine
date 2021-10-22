#include "Fiber.hpp"
#include "FiberPool.hpp"

#ifdef _DEBUG
#include <assert.h>
#endif

using namespace ember::engine::scheduler::fiber;
using namespace ember;

bool Fiber::destroy() {
    DEBUG_ASSERT(handle != ::GetFiberData(), "Dynamic fiber should never call destroy on itself.")
    ::DeleteFiber(handle);
    handle = nullptr;
    return true;
}

bool Fiber::destroy_external() {
    ::DeleteFiber(handle);
    handle = nullptr;
    return true;
}

void Fiber::yield() {

    auto* next { parent };

    DEBUG_ASSERT(next != nullptr, "Dynamic Fiber requires parent to not terminate underlying thread.")
    DEBUG_ASSERT(this != next, "Current fiber and next fiber must never be the same.")

    parent = nullptr;

    /**
     *
     */
    ::SwitchToFiber(next);
}

void self::yield() {
    #ifdef _DEBUG
    auto* entry = ::GetFiberData();
    DEBUG_ASSERT(entry != nullptr, "")
    static_cast<ptr<Fiber>>(entry)->yield();
    #else
    static_cast<ptr<Fiber>>(::GetFiberData())->yield();
    #endif
}

constexpr u64 engine::scheduler::fiber::default_fiber_stack_size() noexcept {
    return 4096ui64;
}
