#pragma once
#include <Engine.Assets/Types/Image.hpp>

namespace ember::game::assets::image {

    class ForestGround01Specular :
        public engine::assets::Image {
    public:
        inline static const asset_guid guid = generate_asset_guid();

    public:
        ForestGround01Specular() :
            Image(ForestGround01Specular::guid) {}
    };

}
