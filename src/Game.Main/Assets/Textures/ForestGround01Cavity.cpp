#include "../Images/ForestGround01Cavity.hpp"

#include <Ember/TextureFormat.hpp>

#include "ForestGound01Cavity.hpp"

using namespace ember::game::assets::texture;
using namespace ember;

const asset_guid ForestGround01Cavity::guid = generate_asset_guid();

ForestGround01Cavity::ForestGround01Cavity() :
    TextureAsset(
        ForestGround01Cavity::guid,
        game::assets::image::ForestGround01Cavity::guid,
        {},
        {},
        TextureFormat::eR8G8B8Unorm,
        0,
        engine::gfx::TextureType::e2d
    ) { }
