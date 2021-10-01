#pragma once
#include <Engine.Assets/Types/Image.hpp>

namespace ember::game::assets::image {

    class GrassWild01AO :
        public engine::assets::Image {
    public:
        inline static const asset_guid guid = generate_asset_guid();

    public:
        GrassWild01AO() :
            Image(GrassWild01AO::guid) {}
    };

}
