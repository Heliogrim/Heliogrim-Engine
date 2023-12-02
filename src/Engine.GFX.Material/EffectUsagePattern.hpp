#pragma once
#include <Engine.Common/Collection/InlineAutoArray.hpp>
#include <Engine.GFX.Render.Predefined/Buffer/ViewBuffer.hpp>

namespace hg::engine::gfx::material {
    struct EffectUsagePattern {
        /**/
        InlineAutoArray<render::ViewBuffer> views;

        /**/
    };
}
