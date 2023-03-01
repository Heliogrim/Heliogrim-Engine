#pragma once

#include "../Texture/Texture.hpp"

namespace hg::engine::gfx::render {
    struct HORenderPassChanges {
        sptr<Texture> target;
    };
}
