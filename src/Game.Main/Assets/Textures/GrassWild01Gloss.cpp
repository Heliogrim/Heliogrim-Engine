#include "GrassWild01Gloss.hpp"
#include "../Images/GrassWild01Gloss.hpp"

#include <Ember/TextureFormat.hpp>

using namespace ember::game::assets::texture;
using namespace ember;

const asset_guid GrassWild01Gloss::guid = generate_asset_guid();

GrassWild01Gloss::GrassWild01Gloss() :
    Texture(
        GrassWild01Gloss::guid,
        game::assets::image::GrassWild01Gloss::guid,
        {},
        {},
        TextureFormat::eR8G8B8Unorm,
        0,
        engine::gfx::TextureType::e2d
    ) { }
