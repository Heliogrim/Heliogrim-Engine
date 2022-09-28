#include "Stick01Normal.hpp"
#include "../Images/Stick01Normal.hpp"

#include <Ember/TextureFormat.hpp>

using namespace ember::game::assets;
using namespace ember;

texture::Stick01Normal::Stick01Normal() :
    TextureAsset(
        texture::Stick01Normal::auto_guid(),
        image::Stick01Normal::auto_guid(),
        {},
        { 8192ui32, 8192ui32, 1ui32 },
        TextureFormat::eR32G32B32A32Sfloat,
        14ui32,
        engine::gfx::TextureType::e2d
    ) { }
