#include "../Images/ForestGround01Normal.hpp"

#include <Ember/TextureFormat.hpp>

#include "ForestGound01Normal.hpp"

using namespace ember::game::assets::texture;
using namespace ember;

const asset_guid ForestGround01Normal::guid = generate_asset_guid();

ForestGround01Normal::ForestGround01Normal() :
    Texture(
        ForestGround01Normal::guid,
        game::assets::image::ForestGround01Normal::guid,
        {},
        {},
        TextureFormat::eR8G8B8Unorm,
        0,
        engine::gfx::TextureType::e2d
    ) { }
