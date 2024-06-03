#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class DefaultDiffuse :
        public TextureAsset,
        public AssetAutoGuid<DefaultDiffuse> {
    public:
        inline static constexpr type_id typeId { "texture::DefaultDiffuse"_typeId };

    public:
        DefaultDiffuse();
    };
}
