#include "../Images/ForestGround01AO.hpp"

#include <Ember/TextureFormat.hpp>

#include "ForestGound01AO.hpp"

using namespace ember::game::assets::texture;
using namespace ember;

ForestGround01AO::ForestGround01AO() :
    TextureAsset(
        ForestGround01AO::auto_guid(),
        game::assets::image::ForestGround01AO::auto_guid(),
        {},
        { 0, 0, 0 },
        TextureFormat::eR8G8B8Unorm,
        0,
        engine::gfx::TextureType::e2d
    ) {}
