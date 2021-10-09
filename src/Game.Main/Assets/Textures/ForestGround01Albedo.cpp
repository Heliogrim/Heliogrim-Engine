#include "../Images/ForestGround01Albedo.hpp"

#include <Ember/TextureFormat.hpp>

#include "ForestGound01Albedo.hpp"

using namespace ember::game::assets::texture;
using namespace ember;

const asset_guid ForestGround01Albedo::guid = generate_asset_guid();

ForestGround01Albedo::ForestGround01Albedo() :
    TextureAsset(
        ForestGround01Albedo::guid,
        game::assets::image::ForestGround01Albedo::guid,
        {},
        { 0, 0, 0 },
        TextureFormat::eR8G8B8Unorm,
        0,
        engine::gfx::TextureType::e2d
    ) { }
