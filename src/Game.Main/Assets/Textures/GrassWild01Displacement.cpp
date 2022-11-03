#include "GrassWild01Displacement.hpp"
#include "../Images/GrassWild01Displacement.hpp"

#include <Ember/TextureFormat.hpp>

using namespace ember::game::assets::texture;
using namespace ember;

GrassWild01Displacement::GrassWild01Displacement() :
    TextureAsset(
        GrassWild01Displacement::unstable_auto_guid(),
        game::assets::image::GrassWild01Displacement::unstable_auto_guid(),
        {},
        {},
        TextureFormat::eR8G8B8Unorm,
        0,
        engine::gfx::TextureType::e2d
    ) { }
