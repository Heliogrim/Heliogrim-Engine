#include "DryGroundRocks01AO.hpp"
#include "../Images/DryGroundRocks01AO.hpp"

#include <Heliogrim/TextureFormat.hpp>

using namespace hg::game::assets;
using namespace hg;

texture::DryGroundRocks01AO::DryGroundRocks01AO() :
    TextureAsset(
        texture::DryGroundRocks01AO::unstable_auto_guid(),
        image::DryGroundRocks01AO::unstable_auto_guid(),
        {},
        { 8192ui32, 8192ui32, 1ui32 },
        TextureFormat::eR16Sfloat,
        14ui32,
        engine::gfx::TextureType::e2d
    ) { }
