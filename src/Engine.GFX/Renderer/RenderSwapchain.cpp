#include "RenderSwapchain.hpp"

#include <Engine.Scheduler/Thread/Thread.hpp>

using namespace ember::engine::gfx;
using namespace ember;

RenderSwapchain::RenderSwapchain(const u8 size_) :
    _desired(0ui8),
    _swaps() {

    init(size_);
}

RenderSwapchain::~RenderSwapchain() = default;

void RenderSwapchain::init(const u8 size_) {

    _desired.store(size_);
    for (u8 i = 0; i < size_; ++i) {
        const auto result { _swaps.try_push(u8 { i }) };
        assert(result);
    }
}

u8 RenderSwapchain::desiredSize() const noexcept {
    return _desired;
}

bool RenderSwapchain::acquireNextFrame(ref<RenderInvocationState> state_) {
    u8 result { 0ui8 };
    if (!_swaps.try_pop(result)) {
        return false;
    }

    state_.swapframe = result;
    return true;
}
