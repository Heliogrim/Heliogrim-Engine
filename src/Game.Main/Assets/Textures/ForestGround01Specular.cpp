#include <Ember/TextureFormat.hpp>

#include "ForestGound01Specular.hpp"
#include "../Images/ForestGround01Specular.hpp"

using namespace ember::game::assets::texture;
using namespace ember;

ForestGround01Specular::ForestGround01Specular() :
    TextureAsset(
        ForestGround01Specular::auto_guid(),
        game::assets::image::ForestGround01Specular::auto_guid(),
        {},
        {},
        TextureFormat::eR8G8B8Unorm,
        0,
        engine::gfx::TextureType::e2d
    ) { }
