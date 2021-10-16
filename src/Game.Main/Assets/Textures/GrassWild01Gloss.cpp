#include "GrassWild01Gloss.hpp"
#include "../Images/GrassWild01Gloss.hpp"

#include <Ember/TextureFormat.hpp>

using namespace ember::game::assets::texture;
using namespace ember;

GrassWild01Gloss::GrassWild01Gloss() :
    TextureAsset(
        GrassWild01Gloss::auto_guid(),
        game::assets::image::GrassWild01Gloss::auto_guid(),
        {},
        {},
        TextureFormat::eR8G8B8Unorm,
        0,
        engine::gfx::TextureType::e2d
    ) { }
