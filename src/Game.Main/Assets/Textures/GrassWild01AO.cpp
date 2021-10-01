#include "GrassWild01AO.hpp"
#include "../Images/GrassWild01AO.hpp"

#include <Ember/TextureFormat.hpp>

using namespace ember::game::assets::texture;
using namespace ember;

const asset_guid GrassWild01AO::guid = generate_asset_guid();

GrassWild01AO::GrassWild01AO() :
    Texture(
        GrassWild01AO::guid,
        game::assets::image::GrassWild01AO::guid,
        {},
        {},
        TextureFormat::eR8G8B8Unorm,
        0,
        engine::gfx::TextureType::e2d
    ) { }
