#include <Ember/TextureFormat.hpp>

#include "ForestGound01Specular.hpp"
#include "../Images/ForestGround01Specular.hpp"

using namespace ember::game::assets::texture;
using namespace ember;

const asset_guid ForestGround01Specular::guid = generate_asset_guid();

ForestGround01Specular::ForestGround01Specular() :
    TextureAsset(
        ForestGround01Specular::guid,
        game::assets::image::ForestGround01Specular::guid,
        {},
        {},
        TextureFormat::eR8G8B8Unorm,
        0,
        engine::gfx::TextureType::e2d
    ) { }
