#include "Awaitable.hpp"

using namespace hg::engine::scheduler::fiber;
using namespace hg;

void FiberAwaitable::reset() noexcept {
    mask = FiberAwaitableBits::eUndefined;
    self = nullptr;
    call = nullptr;
}

bool FiberAwaitable::ready() const noexcept {

    switch (mask) {
        case FiberAwaitableBits::eSignal: {
            return signal->test(std::memory_order::consume);
        }
        case FiberAwaitableBits::eSignalCall: {
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
