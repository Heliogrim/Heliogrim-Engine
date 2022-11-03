#include "IconPdf.hpp"
#include "../Images/IconPdf.hpp"

#include <Ember/TextureFormat.hpp>

using namespace ember::game::assets;
using namespace ember;

texture::IconPdf::IconPdf() :
    TextureAsset(
        texture::IconPdf::unstable_auto_guid(),
        image::IconPdf::unstable_auto_guid(),
        {},
        { 144ui32, 144ui32, 1ui32 },
        TextureFormat::eR8G8B8A8Srgb,
        1ui32,
        engine::gfx::TextureType::e2d
    ) { }
