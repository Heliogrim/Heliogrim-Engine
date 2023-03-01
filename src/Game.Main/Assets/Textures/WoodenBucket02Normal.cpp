#include "WoodenBucket02Normal.hpp"
#include "../Images/WoodenBucket02Normal.hpp"

#include <Heliogrim/TextureFormat.hpp>

using namespace hg::game::assets;
using namespace hg;

texture::WoodenBucket02Normal::WoodenBucket02Normal() :
    TextureAsset(
        texture::WoodenBucket02Normal::unstable_auto_guid(),
        image::WoodenBucket02Normal::unstable_auto_guid(),
        {},
        { 8192ui32, 8192ui32, 1ui32 },
        TextureFormat::eR8G8B8A8Unorm,
        14ui32,
        engine::gfx::TextureType::e2d
    ) { }
