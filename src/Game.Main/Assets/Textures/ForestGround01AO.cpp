#include "ForestGround01AO.hpp"
#include "../Images/ForestGround01AO.hpp"

#include <Ember/TextureFormat.hpp>

using namespace ember::game::assets;
using namespace ember;

texture::ForestGround01AO::ForestGround01AO() :
    TextureAsset(
        texture::ForestGround01AO::unstable_auto_guid(),
        image::ForestGround01AO::unstable_auto_guid(),
        {},
        { 8192ui32, 8192ui32, 1ui32 },
        TextureFormat::eR8Unorm,
        14ui32,
        engine::gfx::TextureType::e2d
    ) { }
