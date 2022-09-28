#include "WoodenBucket02Normal.hpp"
#include "../Images/WoodenBucket02Normal.hpp"

#include <Ember/TextureFormat.hpp>

using namespace ember::game::assets;
using namespace ember;

texture::WoodenBucket02Normal::WoodenBucket02Normal() :
    TextureAsset(
        texture::WoodenBucket02Normal::auto_guid(),
        image::WoodenBucket02Normal::auto_guid(),
        {},
        { 8192ui32, 8192ui32, 1ui32 },
        TextureFormat::eR32G32B32A32Sfloat,
        14ui32,
        engine::gfx::TextureType::e2d
    ) { }
