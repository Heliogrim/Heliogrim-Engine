#include "../Images/ForestGround01Albedo.hpp"

#include <Ember/TextureFormat.hpp>

#include "ForestGound01Albedo.hpp"

using namespace ember::game::assets;
using namespace ember;

texture::ForestGround01Albedo::ForestGround01Albedo() :
    TextureAsset(
        texture::ForestGround01Albedo::auto_guid(),
        game::assets::image::ForestGround01Albedo::auto_guid(),
        {},
        { 0, 0, 0 },
        TextureFormat::eR8G8B8Unorm,
        0,
        engine::gfx::TextureType::e2d
    ) { }
