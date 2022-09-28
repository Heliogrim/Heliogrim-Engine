#include "ForestGround01Normal.hpp"
#include "../Images/ForestGround01Normal.hpp"

#include <Ember/TextureFormat.hpp>

using namespace ember::game::assets;
using namespace ember;

texture::ForestGround01Normal::ForestGround01Normal() :
    TextureAsset(
        texture::ForestGround01Normal::auto_guid(),
        image::ForestGround01Normal::auto_guid(),
        {},
        { 8192ui32, 8192ui32, 1ui32 },
        TextureFormat::eR8G8B8A8Unorm,
        14ui32,
        engine::gfx::TextureType::e2d
    ) { }
