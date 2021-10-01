#pragma once
#include <Engine.Assets/Types/GfxMaterial.hpp>

namespace ember::game::assets::material {

    class GrassWild01 :
        public engine::assets::GfxMaterial {
    public:
        static const asset_guid guid;

    public:
        GrassWild01();
    };
}
