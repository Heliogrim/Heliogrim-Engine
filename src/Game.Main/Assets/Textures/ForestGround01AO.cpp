#include "../Images/ForestGround01AO.hpp"

#include <Ember/TextureFormat.hpp>

#include "ForestGound01AO.hpp"

using namespace ember::game::assets::texture;
using namespace ember;

const asset_guid ForestGround01AO::guid = generate_asset_guid();

ForestGround01AO::ForestGround01AO() :
    Texture(
        ForestGround01AO::guid,
        game::assets::image::ForestGround01AO::guid,
        {},
        { 0, 0, 0 },
        TextureFormat::eR8G8B8Unorm,
        0,
        engine::gfx::TextureType::e2d
    ) {}
