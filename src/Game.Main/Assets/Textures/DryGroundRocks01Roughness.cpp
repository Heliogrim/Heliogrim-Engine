#include "DryGroundRocks01Roughness.hpp"
#include "../Images/DryGroundRocks01Roughness.hpp"

#include <Ember/TextureFormat.hpp>

using namespace ember::game::assets;
using namespace ember;

texture::DryGroundRocks01Roughness::DryGroundRocks01Roughness() :
    TextureAsset(
        texture::DryGroundRocks01Roughness::auto_guid(),
        image::DryGroundRocks01Roughness::auto_guid(),
        {},
        { 8192ui32, 8192ui32, 1ui32 },
        TextureFormat::eR16Sfloat,
        4ui32,
        engine::gfx::TextureType::e2d
    ) { }
