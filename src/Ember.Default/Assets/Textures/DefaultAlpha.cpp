#include "DefaultAlpha.hpp"
#include "../Images/DefaultAlpha.hpp"

#include <Ember/TextureFormat.hpp>

using namespace ember::game::assets;
using namespace ember;

texture::DefaultAlpha::DefaultAlpha() :
    TextureAsset(
        texture::DefaultAlpha::auto_guid(),
        image::DefaultAlpha::auto_guid(),
        {},
        { 1ui32, 1ui32, 1ui32 },
        TextureFormat::eR16Sfloat,
        1ui32,
        engine::gfx::TextureType::e2d
    ) { }
