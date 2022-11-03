#include "IconFavicon.hpp"
#include "../Images/IconFavicon.hpp"

#include <Ember/TextureFormat.hpp>

using namespace ember::game::assets;
using namespace ember;

texture::IconFavicon::IconFavicon() :
    TextureAsset(
        texture::IconFavicon::unstable_auto_guid(),
        image::IconFavicon::unstable_auto_guid(),
        {},
        { 144ui32, 144ui32, 1ui32 },
        TextureFormat::eR8G8B8A8Srgb,
        1ui32,
        engine::gfx::TextureType::e2d
    ) { }
