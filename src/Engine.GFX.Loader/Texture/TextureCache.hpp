#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.Resource/Loader/Cache.hpp>
#include <Engine.GFX/Cache/__fwd.hpp>

#include "Traits.hpp"

namespace ember::engine::gfx::loader {
    class TextureCache final :
        public resource::loader::Cache<assets::Texture, TextureResource> {
    public:
        using this_type = TextureCache;
        using underlying_type = resource::loader::Cache<assets::Texture, TextureResource>;

        using underlying_type::loader_traits;
        using underlying_type::request_type;
        using underlying_type::response_type;

    public:
        TextureCache(
            const non_owning_rptr<cache::GlobalCacheCtrl> cache_
        );

        ~TextureCache() override = default;

    public:
        [[nodiscard]] typename response_type::type operator()(
            _In_ mref<typename request_type::type> request_,
            _In_ mref<typename request_type::options> options_,
            _In_ ref<next_type> next_
        ) const override {
            return next_(_STD move(request_), _STD move(options_));
        }

        [[nodiscard]] virtual typename response_type::type operator()(
            _In_ mref<typename request_type::type> request_,
            _In_ mref<typename request_type::options> options_,
            _In_ mref<typename request_type::stream> streamOptions_,
            _In_ ref<next_type> next_
        ) const override {
            // TODO:
            return next_(_STD move(request_), _STD move(options_), _STD move(streamOptions_));
        }
    };
}
