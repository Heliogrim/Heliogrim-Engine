#include "GrassWild01Normal.hpp"
#include "../Images/GrassWild01Normal.hpp"

#include <Ember/TextureFormat.hpp>

using namespace ember::game::assets::texture;
using namespace ember;

const asset_guid GrassWild01Normal::guid = generate_asset_guid();

GrassWild01Normal::GrassWild01Normal() :
    Texture(
        GrassWild01Normal::guid,
        game::assets::image::GrassWild01Normal::guid,
        {},
        {},
        TextureFormat::eR8G8B8Unorm,
        0,
        engine::gfx::TextureType::e2d
    ) { }
