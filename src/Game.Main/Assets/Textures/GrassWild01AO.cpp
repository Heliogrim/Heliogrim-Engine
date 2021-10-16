#include "GrassWild01AO.hpp"
#include "../Images/GrassWild01AO.hpp"

#include <Ember/TextureFormat.hpp>

using namespace ember::game::assets::texture;
using namespace ember;

GrassWild01AO::GrassWild01AO() :
    TextureAsset(
        GrassWild01AO::auto_guid(),
        game::assets::image::GrassWild01AO::auto_guid(),
        {},
        {},
        TextureFormat::eR8G8B8Unorm,
        0,
        engine::gfx::TextureType::e2d
    ) { }
