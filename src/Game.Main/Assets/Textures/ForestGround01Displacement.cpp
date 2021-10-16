#include "../Images/ForestGround01Displacement.hpp"

#include <Ember/TextureFormat.hpp>

#include "ForestGound01Displacement.hpp"

using namespace ember::game::assets::texture;
using namespace ember;

ForestGround01Displacement::ForestGround01Displacement() :
    TextureAsset(
        ForestGround01Displacement::auto_guid(),
        game::assets::image::ForestGround01Displacement::auto_guid(),
        {},
        {},
        TextureFormat::eR8G8B8Unorm,
        0,
        engine::gfx::TextureType::e2d
    ) { }
