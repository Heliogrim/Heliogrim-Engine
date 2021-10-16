#include <Ember/TextureFormat.hpp>

#include "ForestGound01Roughness.hpp"
#include "../Images/ForestGround01Roughness.hpp"

using namespace ember::game::assets::texture;
using namespace ember;

ForestGround01Roughness::ForestGround01Roughness() :
    TextureAsset(
        ForestGround01Roughness::auto_guid(),
        game::assets::image::ForestGround01Roughness::auto_guid(),
        {},
        {},
        TextureFormat::eR8G8B8Unorm,
        0,
        engine::gfx::TextureType::e2d
    ) { }
