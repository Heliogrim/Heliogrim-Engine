#pragma once
#include <Engine.Assets/Types/Texture.hpp>
#include <Engine.Resource/Loader/Loader.hpp>

#include "TextureLoader.hpp"

namespace ember::engine::gfx {

    class RevTextureLoader :
        public loader::TextureLoader,
        public res::Loader<assets::Texture> {
    public:
        using this_type = RevTextureLoader;
        using loader_type = res::Loader<assets::Texture>;

        using result_type = loader_type::result_type;
        using options_type = loader_type::options_type;

    public:
        RevTextureLoader(cref<sptr<Device>> device_);

    public:
        [[nodiscard]] result_type operator()(const ptr<assets::Texture> asset_, options_type options_) override;
    };
}
