#include "Stick01AO.hpp"
#include "../Images/Stick01AO.hpp"

#include <Ember/TextureFormat.hpp>

using namespace ember::game::assets;
using namespace ember;

texture::Stick01AO::Stick01AO() :
    TextureAsset(
        texture::Stick01AO::auto_guid(),
        image::Stick01AO::auto_guid(),
        {},
        { 4096ui32, 4096ui32, 1ui32 },
        TextureFormat::eR16Sfloat,
        12ui32,
        engine::gfx::TextureType::e2d
    ) { }
