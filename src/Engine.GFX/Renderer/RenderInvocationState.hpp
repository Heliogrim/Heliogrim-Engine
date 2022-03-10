#pragma once
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/BytellHashMap.hpp>

#include "../Framebuffer/Framebuffer.hpp"
#include "../Command/CommandBatch.hpp"

namespace ember::engine::gfx {

    struct RenderInvocationState {
        ptr<Framebuffer> framebuffer;

        [[deprecated]] u8 swapframe;

        ska::bytell_hash_map<_STD string_view, sptr<void>> data;
    };
}
