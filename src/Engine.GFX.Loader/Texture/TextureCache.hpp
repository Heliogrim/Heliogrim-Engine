#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.Resource/Loader/Cache.hpp>
#include <Engine.GFX/Cache/__fwd.hpp>
#include <Engine.GFX/Cache/CacheResult.hpp>

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

        using cache_key_type = const non_owning_rptr<const assets::Texture>;
        using cache_value_type = smr<TextureResource>;
        using cache_result_type = cache::Result<cache::QueryResultType, cache_value_type>;

    public:
        TextureCache(
            const non_owning_rptr<cache::GlobalCacheCtrl> cacheCtrl_
        );

        ~TextureCache() override = default;

    private:
        const non_owning_rptr<cache::GlobalCacheCtrl> _cacheCtrl;

    public:
        [[nodiscard]] bool contains(const non_owning_rptr<const assets::Texture> asset_) const noexcept;

        [[nodiscard]] cache::Result<cache::QueryResultType, smr<TextureResource>> query(
            const non_owning_rptr<const assets::Texture> asset_
        ) const noexcept;

    public:
        bool store(
            const non_owning_rptr<const assets::Texture> asset_,
            _In_ mref<smr<TextureResource>> resource_
        ) const noexcept;

        [[nodiscard]] bool remove(const non_owning_rptr<const assets::Texture> asset_) noexcept;

        [[nodiscard]] bool remove(
            const non_owning_rptr<const assets::Texture> asset_,
            _Out_ ref<smr<TextureResource>> resource_
        ) noexcept;

        void clear();

    public:
        [[nodiscard]] typename response_type::type operator()(
            _In_ mref<typename request_type::type> request_,
            _In_ mref<typename request_type::options> options_,
            _In_ cref<next_type> next_
        ) const override;

        [[nodiscard]] typename response_type::type operator()(
            _In_ mref<typename request_type::type> request_,
            _In_ mref<typename request_type::options> options_,
            _In_ mref<typename request_type::stream> streamOptions_,
            _In_ cref<next_type> next_
        ) const override;
    };
}
