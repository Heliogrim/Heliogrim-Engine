#pragma once
#include <Engine.Assets/Types/Texture.hpp>

namespace ember::game::assets::texture {

    class ForestGround01Roughness :
        public engine::assets::Texture {
    public:
        static const asset_guid guid;

    public:
        ForestGround01Roughness();
    };

}
