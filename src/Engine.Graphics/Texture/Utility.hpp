#pragma once

#include "Texture.hpp"

namespace hg::engine::gfx::texture {
    [[nodiscard]] bool is2d(const Texture& texture_);

    [[nodiscard]] bool is3d(const Texture& texture_);

    [[nodiscard]] bool isSquare(const Texture& texture_);

    [[nodiscard]] bool isPow2(const Texture& texture_);
}
