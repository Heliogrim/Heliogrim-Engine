#pragma once
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/BytellHashMap.hpp>

#include <Engine.GFX/Framebuffer/Framebuffer.hpp>
#include <Engine.GFX/Shader/DiscreteBindingCache.hpp>
#include <Engine.GFX/Cache/LocalCacheCtrl.hpp>
#include <Engine.GFX/Memory/LocalPooledAllocator.hpp>

namespace hg::engine::gfx::render {
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
