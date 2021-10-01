#pragma once
#include <Engine.Assets/Types/Image.hpp>

namespace ember::game::assets::image {

    class ForestGround01Gloss :
        public engine::assets::Image {
    public:
        inline static const asset_guid guid = generate_asset_guid();

    public:
        ForestGround01Gloss() :
            Image(ForestGround01Gloss::guid) {}
    };

}
