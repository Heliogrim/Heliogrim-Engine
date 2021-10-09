#include "GrassWild01Cavity.hpp"
#include "../Images/GrassWild01Cavity.hpp"

#include "Ember/TextureFormat.hpp"

using namespace ember::game::assets::texture;
using namespace ember;

const asset_guid GrassWild01Cavity::guid = generate_asset_guid();

GrassWild01Cavity::GrassWild01Cavity() :
    TextureAsset(
        GrassWild01Cavity::guid,
        game::assets::image::GrassWild01Cavity::guid,
        {},
        {},
        TextureFormat::eR8G8B8Unorm,
        0,
        engine::gfx::TextureType::e2d
    ) { }
