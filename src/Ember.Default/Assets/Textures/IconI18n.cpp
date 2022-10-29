#include "IconI18n.hpp"
#include "../Images/IconI18n.hpp"

#include <Ember/TextureFormat.hpp>

using namespace ember::game::assets;
using namespace ember;

texture::IconI18n::IconI18n() :
    TextureAsset(
        texture::IconI18n::auto_guid(),
        image::IconI18n::auto_guid(),
        {},
        { 144ui32, 144ui32, 1ui32 },
        TextureFormat::eR8G8B8A8Srgb,
        1ui32,
        engine::gfx::TextureType::e2d
    ) { }
