#include "DefaultNormal.hpp"
#include "../Images/DefaultNormal.hpp"

#include <Ember/TextureFormat.hpp>

using namespace ember::game::assets;
using namespace ember;

texture::DefaultNormal::DefaultNormal() :
    TextureAsset(
        texture::DefaultNormal::unstable_auto_guid(),
        image::DefaultNormal::unstable_auto_guid(),
        {},
        { 1ui32, 1ui32, 1ui32 },
        TextureFormat::eR32G32B32A32Sfloat,
        1ui32,
        engine::gfx::TextureType::e2d
    ) { }
