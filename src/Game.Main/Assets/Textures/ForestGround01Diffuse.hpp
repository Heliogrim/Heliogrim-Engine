#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class ForestGround01Diffuse :
        public TextureAsset,
        public AssetAutoGuid<ForestGround01Diffuse> {
    public:
        inline static constexpr type_id typeId { "texture::ForestGround01Diffuse"_typeId };

    public:
        ForestGround01Diffuse();
    };
}
