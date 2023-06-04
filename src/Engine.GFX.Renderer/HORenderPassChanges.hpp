#pragma once

#include <Engine.GFX/Texture/Texture.hpp>

namespace hg::engine::gfx::render {
    struct HORenderPassChanges {
        sptr<Texture> target;
    };
}
