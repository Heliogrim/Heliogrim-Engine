#include "GrassWild01Albedo.hpp"
#include "../Images/GrassWild01Albedo.hpp"

#include <Ember/TextureFormat.hpp>

using namespace ember::game::assets::texture;
using namespace ember;

const asset_guid GrassWild01Albedo::guid = generate_asset_guid();

GrassWild01Albedo::GrassWild01Albedo() :
    Texture(
        GrassWild01Albedo::guid,
        game::assets::image::GrassWild01Albedo::guid,
        {},
        {},
        TextureFormat::eR8G8B8Unorm,
        0,
        engine::gfx::TextureType::e2d
    ) { }
