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
        TextureFormat::eR32G32B32A32Sfloat,
        4ui32,
        engine::gfx::TextureType::e2d
    ) { }
