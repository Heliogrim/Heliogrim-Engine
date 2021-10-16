#include "GrassWild01Albedo.hpp"
#include "../Images/GrassWild01Albedo.hpp"

#include <Ember/TextureFormat.hpp>

using namespace ember::game::assets::texture;
using namespace ember;

GrassWild01Albedo::GrassWild01Albedo() :
    TextureAsset(
        GrassWild01Albedo::auto_guid(),
        game::assets::image::GrassWild01Albedo::auto_guid(),
        {},
        {},
        TextureFormat::eR8G8B8Unorm,
        0,
        engine::gfx::TextureType::e2d
    ) { }
