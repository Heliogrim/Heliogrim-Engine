#include "ForestGround01Roughness.hpp"
#include "../Images/ForestGround01Roughness.hpp"

#include <Heliogrim/TextureFormat.hpp>

using namespace hg::game::assets;
using namespace hg;

texture::ForestGround01Roughness::ForestGround01Roughness() :
    TextureAsset(
        texture::ForestGround01Roughness::unstable_auto_guid(),
        image::ForestGround01Roughness::unstable_auto_guid(),
        {},
        { 8192ui32, 8192ui32, 1ui32 },
        TextureFormat::eR8Unorm,
        14ui32,
        engine::gfx::TextureType::e2d
    ) { }
