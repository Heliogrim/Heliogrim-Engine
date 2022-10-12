#include "DefaultMetalness.hpp"
#include "../Images/DefaultMetalness.hpp"

#include <Ember/TextureFormat.hpp>

using namespace ember::game::assets;
using namespace ember;

texture::DefaultMetalness::DefaultMetalness() :
    TextureAsset(
        texture::DefaultMetalness::auto_guid(),
        image::DefaultMetalness::auto_guid(),
        {},
        { 1ui32, 1ui32, 1ui32 },
        TextureFormat::eR16Sfloat,
        1ui32,
        engine::gfx::TextureType::e2d
    ) { }
