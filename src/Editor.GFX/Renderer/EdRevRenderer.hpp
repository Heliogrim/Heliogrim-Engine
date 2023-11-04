#pragma once

#include <Engine.GFX.Render/Renderer.hpp>

namespace hg::editor::gfx {
    class EdRevRenderer :
        public ::hg::engine::gfx::render::Renderer {
    public:
        using this_type = EdRevRenderer;

    public:
        EdRevRenderer() noexcept;

        ~EdRevRenderer() noexcept override = default;
    };
}
