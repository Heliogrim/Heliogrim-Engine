#pragma once

#include "../Texture/Texture.hpp"

namespace ember::engine::gfx::render {
    struct HORenderPassChanges {
        sptr<Texture> target;
    };
}
