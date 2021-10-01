#pragma once
#include <Engine.Assets/Types/Image.hpp>

namespace ember::game::assets::image {

    class GrassWild01Specular :
        public engine::assets::Image {
    public:
        inline static const asset_guid guid = generate_asset_guid();

    public:
        GrassWild01Specular() :
            Image(GrassWild01Specular::guid) {}
    };

}
