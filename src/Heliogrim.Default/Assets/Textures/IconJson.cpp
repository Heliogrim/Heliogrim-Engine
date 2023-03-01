#include "IconJson.hpp"
#include "../Images/IconJson.hpp"

#include <Heliogrim/TextureFormat.hpp>

using namespace hg::game::assets;
using namespace hg;

texture::IconJson::IconJson() :
    TextureAsset(
        texture::IconJson::unstable_auto_guid(),
        image::IconJson::unstable_auto_guid(),
        {},
        { 144ui32, 144ui32, 1ui32 },
        TextureFormat::eR8G8B8A8Srgb,
        1ui32,
        engine::gfx::TextureType::e2d
    ) { }
