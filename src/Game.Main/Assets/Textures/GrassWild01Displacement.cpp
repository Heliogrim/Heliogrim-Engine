#include "GrassWild01Displacement.hpp"
#include "../Images/GrassWild01Displacement.hpp"

#include <Ember/TextureFormat.hpp>

using namespace ember::game::assets::texture;
using namespace ember;

const asset_guid GrassWild01Displacement::guid = generate_asset_guid();

GrassWild01Displacement::GrassWild01Displacement() :
    TextureAsset(
        GrassWild01Displacement::guid,
        game::assets::image::GrassWild01Displacement::guid,
        {},
        {},
        TextureFormat::eR8G8B8Unorm,
        0,
        engine::gfx::TextureType::e2d
    ) { }
