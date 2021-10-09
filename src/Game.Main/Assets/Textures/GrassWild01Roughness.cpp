#include "GrassWild01Roughness.hpp"
#include "../Images/GrassWild01Roughness.hpp"

#include <Ember/TextureFormat.hpp>

using namespace ember::game::assets::texture;
using namespace ember;

const asset_guid GrassWild01Roughness::guid = generate_asset_guid();

GrassWild01Roughness::GrassWild01Roughness() :
    TextureAsset(
        GrassWild01Roughness::guid,
        game::assets::image::GrassWild01Roughness::guid,
        {},
        {},
        TextureFormat::eR8G8B8Unorm,
        0,
        engine::gfx::TextureType::e2d
    ) { }
