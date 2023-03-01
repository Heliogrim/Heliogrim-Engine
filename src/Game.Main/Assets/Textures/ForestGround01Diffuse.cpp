#include "ForestGround01Diffuse.hpp"
#include "../Images/ForestGround01Diffuse.hpp"

#include <Heliogrim/TextureFormat.hpp>

using namespace hg::game::assets;
using namespace hg;

texture::ForestGround01Diffuse::ForestGround01Diffuse() :
    TextureAsset(
        texture::ForestGround01Diffuse::auto_guid(),
        image::ForestGround01Diffuse::unstable_auto_guid(),
        {},
        { 8192ui32, 8192ui32, 1ui32 },
        TextureFormat::eR8G8B8A8Unorm,
        14ui32,
        engine::gfx::TextureType::e2d
    ) { }
