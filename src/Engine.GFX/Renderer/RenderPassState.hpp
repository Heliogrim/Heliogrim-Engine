#pragma once
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/BytellHashMap.hpp>

#include "../Framebuffer/Framebuffer.hpp"
#include "../Shader/DiscreteBindingCache.hpp"

namespace ember::engine::gfx::render {

    struct RenderPassState {
        /**
         * Final Render Target
         */
        ptr<Framebuffer> framebuffer;

        /**
         * Resource Descriptors
         */
        DiscreteBindingCache bindingCache;

        /**
         * Unstructured Data
         */
        ska::bytell_hash_map<_STD string_view, sptr<void>> data;
    };
}
