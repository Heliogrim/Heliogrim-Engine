#include "../Images/ForestGround01Normal.hpp"

#include <Ember/TextureFormat.hpp>

#include "ForestGround01Normal.hpp"

using namespace ember::game::assets::texture;
using namespace ember;

ForestGround01Normal::ForestGround01Normal() :
    TextureAsset(
        ForestGround01Normal::auto_guid(),
        game::assets::image::ForestGround01Normal::auto_guid(),
        {},
        {},
        TextureFormat::eR8G8B8Unorm,
        0,
        engine::gfx::TextureType::e2d
    ) { }
