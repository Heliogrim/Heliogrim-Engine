#pragma once
#include <Engine.Assets/Types/GfxMaterial.hpp>

namespace ember::game::assets::material {

    class ForestGround01 :
        public engine::assets::GfxMaterial {
    public:
        static const asset_guid guid;

    public:
        ForestGround01();
    };
}
