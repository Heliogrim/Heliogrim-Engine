#include "../Images/ForestGround01Cavity.hpp"

#include <Ember/TextureFormat.hpp>

#include "ForestGound01Cavity.hpp"

using namespace ember::game::assets::texture;
using namespace ember;

ForestGround01Cavity::ForestGround01Cavity() :
    TextureAsset(
        ForestGround01Cavity::auto_guid(),
        game::assets::image::ForestGround01Cavity::auto_guid(),
        {},
        {},
        TextureFormat::eR8G8B8Unorm,
        0,
        engine::gfx::TextureType::e2d
    ) { }
