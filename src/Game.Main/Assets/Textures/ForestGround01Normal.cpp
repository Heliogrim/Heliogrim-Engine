#include "ForestGround01Normal.hpp"
#include "../Images/ForestGround01Normal.hpp"

#include <Heliogrim/TextureFormat.hpp>

using namespace hg::game::assets;
using namespace hg;

texture::ForestGround01Normal::ForestGround01Normal() :
    TextureAsset(
        texture::ForestGround01Normal::unstable_auto_guid(),
        image::ForestGround01Normal::unstable_auto_guid(),
        {},
        { 8192ui32, 8192ui32, 1ui32 },
        TextureFormat::eR8G8B8A8Unorm,
        14ui32,
        engine::gfx::TextureType::e2d
    ) { }
