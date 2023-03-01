#include "WoodenBucket02Roughness.hpp"
#include "../Images/WoodenBucket02Roughness.hpp"

#include <Heliogrim/TextureFormat.hpp>

using namespace hg::game::assets;
using namespace hg;

texture::WoodenBucket02Roughness::WoodenBucket02Roughness() :
    TextureAsset(
        texture::WoodenBucket02Roughness::unstable_auto_guid(),
        image::WoodenBucket02Roughness::unstable_auto_guid(),
        {},
        { 8192ui32, 8192ui32, 1ui32 },
        TextureFormat::eR8Unorm,
        14ui32,
        engine::gfx::TextureType::e2d
    ) { }
