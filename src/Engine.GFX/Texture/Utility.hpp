#pragma once

#include "Texture.hpp"

namespace ember::engine::gfx::texture {

    [[nodiscard]] bool is2d(const Texture& texture_);

    [[nodiscard]] bool is3d(const Texture& texture_);

    [[nodiscard]] bool isSquare(const Texture& texture_);

    [[nodiscard]] bool isPow2(const Texture& texture_);

}
