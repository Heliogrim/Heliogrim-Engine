#include "ForestGround01AO.hpp"
#include "../Images/ForestGround01AO.hpp"

#include <Heliogrim/TextureFormat.hpp>

using namespace hg::game::assets;
using namespace hg;

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
