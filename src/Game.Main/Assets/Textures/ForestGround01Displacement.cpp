#include "../Images/ForestGround01Displacement.hpp"

#include <Ember/TextureFormat.hpp>

#include "ForestGound01Displacement.hpp"

using namespace ember::game::assets::texture;
using namespace ember;

const asset_guid ForestGround01Displacement::guid = generate_asset_guid();

ForestGround01Displacement::ForestGround01Displacement() :
    Texture(
        ForestGround01Displacement::guid,
        game::assets::image::ForestGround01Displacement::guid,
        {},
        {},
        TextureFormat::eR8G8B8Unorm,
        0,
        engine::gfx::TextureType::e2d
    ) { }
