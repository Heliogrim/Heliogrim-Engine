#include <Ember/TextureFormat.hpp>

#include "ForestGound01Roughness.hpp"
#include "../Images/ForestGround01Roughness.hpp"

using namespace ember::game::assets::texture;
using namespace ember;

const asset_guid ForestGround01Roughness::guid = generate_asset_guid();

ForestGround01Roughness::ForestGround01Roughness() :
    Texture(
        ForestGround01Roughness::guid,
        game::assets::image::ForestGround01Roughness::guid,
        {},
        {},
        TextureFormat::eR8G8B8Unorm,
        0,
        engine::gfx::TextureType::e2d
    ) { }
