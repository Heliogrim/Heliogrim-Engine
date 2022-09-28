#include "Rock01Roughness.hpp"
#include "../Images/Rock01Roughness.hpp"

#include <Ember/TextureFormat.hpp>

using namespace ember::game::assets;
using namespace ember;

texture::Rock01Roughness::Rock01Roughness() :
    TextureAsset(
        texture::Rock01Roughness::auto_guid(),
        image::Rock01Roughness::auto_guid(),
        {},
        { 4096ui32, 4096ui32, 1ui32 },
        TextureFormat::eR16Sfloat,
        12ui32,
        engine::gfx::TextureType::e2d
    ) { }
