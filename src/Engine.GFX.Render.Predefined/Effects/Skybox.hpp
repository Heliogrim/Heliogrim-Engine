#pragma once
#include <Engine.GFX.Acc/AccelerationEffect.hpp>

namespace hg::engine::gfx::render {
    [[nodiscard]] extern smr<const acc::AccelerationEffect> makeSkyboxEffect();
}
