#include "Rock01Normal.hpp"
#include "../Images/Rock01Normal.hpp"

#include <Ember/TextureFormat.hpp>

using namespace ember::game::assets;
using namespace ember;

texture::Rock01Normal::Rock01Normal() :
    TextureAsset(
        texture::Rock01Normal::auto_guid(),
        image::Rock01Normal::auto_guid(),
        {},
        { 4096ui32, 4096ui32, 1ui32 },
        TextureFormat::eR32G32B32A32Sfloat,
        12ui32,
        engine::gfx::TextureType::e2d
    ) { }
