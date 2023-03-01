#include "Rock01Roughness.hpp"
#include "../Images/Rock01Roughness.hpp"

#include <Heliogrim/TextureFormat.hpp>

using namespace hg::game::assets;
using namespace hg;

texture::Rock01Roughness::Rock01Roughness() :
    TextureAsset(
        texture::Rock01Roughness::unstable_auto_guid(),
        image::Rock01Roughness::unstable_auto_guid(),
        {},
        { 8192ui32, 8192ui32, 1ui32 },
        TextureFormat::eR8Unorm,
        14ui32,
        engine::gfx::TextureType::e2d
    ) { }
