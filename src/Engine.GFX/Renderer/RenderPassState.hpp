#pragma once
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/BytellHashMap.hpp>

#include "../Framebuffer/Framebuffer.hpp"

namespace ember::engine::gfx::render {

    struct RenderPassState {
        ptr<Framebuffer> framebuffer;

        ska::bytell_hash_map<_STD string_view, sptr<void>> data;
    };
}
