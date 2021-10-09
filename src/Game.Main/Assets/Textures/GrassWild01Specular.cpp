#include "GrassWild01Specular.hpp"
#include "../Images/GrassWild01Specular.hpp"

#include <Ember/TextureFormat.hpp>

using namespace ember::game::assets::texture;
using namespace ember;

const asset_guid GrassWild01Specular::guid = generate_asset_guid();

GrassWild01Specular::GrassWild01Specular() :
    TextureAsset(
        GrassWild01Specular::guid,
        game::assets::image::GrassWild01Specular::guid,
        {},
        {},
        TextureFormat::eR8G8B8Unorm,
        0,
        engine::gfx::TextureType::e2d
    ) { }
