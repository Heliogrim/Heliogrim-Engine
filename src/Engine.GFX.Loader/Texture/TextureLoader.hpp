#pragma once

#include <Engine.GFX/Cache/__fwd.hpp>
#include <Engine.Resource/Loader/LoaderChain.hpp>

#include "TextureResource.hpp"
#include "TextureCache.hpp"
#include "TextureFeedback.hpp"
#include "TextureTransformer.hpp"

#include "__fwd.hpp"

namespace hg::engine::gfx::loader {
    using TextureSourceLoader = ::hg::engine::resource::loader::SourceLoader;

    using TextureLoaderType = resource::loader::LoaderChain<
        assets::TextureAsset,
        TextureResource,
        resource::loader::MetaTypeWrapperList<std::type_identity_t, std::type_identity_t, std::type_identity_t, sptr>,
        TextureCache,
        TextureFeedback,
        TextureTransformer,
        TextureSourceLoader
    >;

    class TextureLoader final :
        public TextureLoaderType {
    public:
        using this_type = TextureLoader;

    public:
        TextureLoader(
            const non_owning_rptr<cache::GlobalCacheCtrl> cache_,
            const non_owning_rptr<pool::GlobalResourcePool> pool_,
            mref<sptr<TextureSourceLoader>> sourceLoader_
        );
    };
}
