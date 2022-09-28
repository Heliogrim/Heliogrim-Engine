#pragma once
#include <Engine.Assets/Types/Texture/Texture.hpp>
#include <Engine.Resource/Loader/StreamLoader.hpp>

#include <Engine.GFX/Cache/__fwd.hpp>

#include "TextureLoader.hpp"

namespace ember::engine::gfx {

    class RevTextureLoader final :
        public loader::TextureLoader,
        public res::StreamLoader<assets::Texture>,
        public _STD enable_shared_from_this<RevTextureLoader> {
    public:
        using this_type = RevTextureLoader;
        using loader_type = res::StreamLoader<assets::Texture>;

        using result_type = loader_type::result_type;
        using options_type = loader_type::options_type;

    public:
        RevTextureLoader(const ptr<cache::GlobalCacheCtrl> cache_);

    public:
        [[nodiscard]] result_type operator()(const ptr<assets::Texture> asset_, options_type options_) override;

    public:
        void streamLoad(const ptr<res::partial::Streamable<res::Resource>> resource_,
            stream_options_type options_) override;

        void streamUnload(const ptr<res::partial::Streamable<res::Resource>> resource_,
            stream_options_type options_) override;
    };
}
