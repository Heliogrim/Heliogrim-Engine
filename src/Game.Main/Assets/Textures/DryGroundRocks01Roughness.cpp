#include "DryGroundRocks01Roughness.hpp"
#include "../Images/DryGroundRocks01Roughness.hpp"

#include <Heliogrim/TextureFormat.hpp>

using namespace hg::game::assets;
using namespace hg;

texture::DryGroundRocks01Roughness::DryGroundRocks01Roughness() :
    TextureAsset(
        texture::DryGroundRocks01Roughness::unstable_auto_guid(),
        image::DryGroundRocks01Roughness::unstable_auto_guid(),
        {},
        { 8192ui32, 8192ui32, 1ui32 },
        TextureFormat::eR16Sfloat,
        14ui32,
        engine::gfx::TextureType::e2d
    ) { }
