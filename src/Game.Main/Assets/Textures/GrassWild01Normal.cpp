#include "GrassWild01Normal.hpp"
#include "../Images/GrassWild01Normal.hpp"

#include <Ember/TextureFormat.hpp>

using namespace ember::game::assets::texture;
using namespace ember;

GrassWild01Normal::GrassWild01Normal() :
    TextureAsset(
        GrassWild01Normal::auto_guid(),
        game::assets::image::GrassWild01Normal::auto_guid(),
        {},
        {},
        TextureFormat::eR8G8B8Unorm,
        0,
        engine::gfx::TextureType::e2d
    ) { }
