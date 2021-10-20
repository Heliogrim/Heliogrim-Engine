#include "../Images/ForestGround01Gloss.hpp"

#include <Ember/TextureFormat.hpp>

#include "ForestGround01Gloss.hpp"

using namespace ember::game::assets::texture;
using namespace ember;

ForestGround01Gloss::ForestGround01Gloss() :
    TextureAsset(
        ForestGround01Gloss::auto_guid(),
        game::assets::image::ForestGround01Gloss::auto_guid(),
        {},
        {},
        TextureFormat::eR8G8B8Unorm,
        0,
        engine::gfx::TextureType::e2d
    ) { }
