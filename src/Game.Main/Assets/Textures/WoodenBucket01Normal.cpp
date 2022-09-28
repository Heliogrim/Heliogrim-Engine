#include "WoodenBucket01Normal.hpp"
#include "../Images/WoodenBucket01Normal.hpp"

#include <Ember/TextureFormat.hpp>

using namespace ember::game::assets;
using namespace ember;

texture::WoodenBucket01Normal::WoodenBucket01Normal() :
    TextureAsset(
        texture::WoodenBucket01Normal::auto_guid(),
        image::WoodenBucket01Normal::auto_guid(),
        {},
        { 8192ui32, 8192ui32, 1ui32 },
        TextureFormat::eR8G8B8A8Unorm,
        14ui32,
        engine::gfx::TextureType::e2d
    ) { }
