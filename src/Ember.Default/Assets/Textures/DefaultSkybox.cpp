#include "DefaultSkybox.hpp"
#include "../Images/DefaultSkybox.hpp"

#include <Ember/TextureFormat.hpp>

using namespace ember::game::assets;
using namespace ember;

texture::DefaultSkybox::DefaultSkybox() :
    TextureAsset(
        texture::DefaultSkybox::unstable_auto_guid(),
        image::DefaultSkybox::unstable_auto_guid(),
        {},
        { 1024ui32, 1024ui32, 1ui32 },
        TextureFormat::eR16G16B16A16Sfloat,
        11ui32,
        engine::gfx::TextureType::eCube
    ) { }
