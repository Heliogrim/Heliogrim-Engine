#include "Dandelion01Roughness.hpp"
#include "../Images/Dandelion01Roughness.hpp"

#include <Heliogrim/TextureFormat.hpp>

using namespace hg::game::assets;
using namespace hg;

texture::Dandelion01Roughness::Dandelion01Roughness() :
    TextureAsset(
        texture::Dandelion01Roughness::unstable_auto_guid(),
        image::Dandelion01Roughness::unstable_auto_guid(),
        {},
        { 8192ui32, 8192ui32, 1ui32 },
        TextureFormat::eR16Sfloat,
        14ui32,
        engine::gfx::TextureType::e2d
    ) { }
