#include "Awaitable.hpp"

using namespace ember::engine::scheduler::fiber;
using namespace ember;

void FiberAwaitable::reset() noexcept {
    mask = FiberAwaitableBits::eUndefined;
    self = nullptr;
    call = nullptr;
}

bool FiberAwaitable::ready() const noexcept {

    switch (mask) {
        case FiberAwaitableBits::eSignal: {
            #ifdef _DEBUG
            __debugbreak();
            #endif
            return signal->test(_STD memory_order::consume);
        }
        case FiberAwaitableBits::eSignalCall: {
            #ifdef _DEBUG
            __debugbreak();
            #endif
            return ((*static_cast<ptr<self_type>>(self)).*call)();
        }
        default: {
            return true;
        }
    }
}

bool FiberAwaitable::operator!() const noexcept {
    return mask == FiberAwaitableBits::eUndefined;
}
