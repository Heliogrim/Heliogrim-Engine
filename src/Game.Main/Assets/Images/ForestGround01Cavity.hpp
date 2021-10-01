#pragma once
#include <Engine.Assets/Types/Image.hpp>

namespace ember::game::assets::image {

    class ForestGround01Cavity :
        public engine::assets::Image {
    public:
        inline static const asset_guid guid = generate_asset_guid();

    public:
        ForestGround01Cavity() :
            Image(ForestGround01Cavity::guid) {}
    };

}
