#include "GrassWild01Roughness.hpp"
#include "../Images/GrassWild01Roughness.hpp"

#include <Ember/TextureFormat.hpp>

using namespace ember::game::assets::texture;
using namespace ember;

GrassWild01Roughness::GrassWild01Roughness() :
    TextureAsset(
        GrassWild01Roughness::auto_guid(),
        game::assets::image::GrassWild01Roughness::auto_guid(),
        {},
        {},
        TextureFormat::eR8G8B8Unorm,
        0,
        engine::gfx::TextureType::e2d
    ) { }
