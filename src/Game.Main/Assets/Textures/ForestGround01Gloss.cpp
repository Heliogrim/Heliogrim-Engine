#include "../Images/ForestGround01Gloss.hpp"

#include <Ember/TextureFormat.hpp>

#include "ForestGound01Gloss.hpp"

using namespace ember::game::assets::texture;
using namespace ember;

const asset_guid ForestGround01Gloss::guid = generate_asset_guid();

ForestGround01Gloss::ForestGround01Gloss() :
    Texture(
        ForestGround01Gloss::guid,
        game::assets::image::ForestGround01Gloss::guid,
        {},
        {},
        TextureFormat::eR8G8B8Unorm,
        0,
        engine::gfx::TextureType::e2d
    ) { }
