#pragma once
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/BytellHashMap.hpp>

#include "../Framebuffer/Framebuffer.hpp"
#include "../Shader/DiscreteBindingCache.hpp"
#include "../Cache/LocalCacheCtrl.hpp"
#include "../Memory/LocalPooledAllocator.hpp"

namespace ember::engine::gfx::render {

    struct RenderPassState {

        ~RenderPassState();

        /**
         *
         */
        sptr<Device> device;

        /**
         * Final Render Target
         */
        ptr<Framebuffer> framebuffer;

        /**
         * Resources
         */
        cache::LocalCacheCtrl cacheCtrl;
        memory::LocalPooledAllocator alloc;

        DiscreteBindingCache bindingCache;

        /**
         * Unstructured Data
         */
        ska::bytell_hash_map<_STD string_view, sptr<void>> data;
    };
}
