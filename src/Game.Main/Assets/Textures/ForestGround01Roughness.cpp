#include "ForestGround01Roughness.hpp"
#include "../Images/ForestGround01Roughness.hpp"

#include <Ember/TextureFormat.hpp>

using namespace ember::game::assets;
using namespace ember;

texture::ForestGround01Roughness::ForestGround01Roughness() :
    TextureAsset(
        texture::ForestGround01Roughness::auto_guid(),
        image::ForestGround01Roughness::auto_guid(),
        {},
        { 8192ui32, 8192ui32, 1ui32 },
        TextureFormat::eR8Unorm,
        14ui32,
        engine::gfx::TextureType::e2d
    ) { }
