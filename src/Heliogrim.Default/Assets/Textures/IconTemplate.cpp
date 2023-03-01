#include "IconTemplate.hpp"
#include "../Images/IconTemplate.hpp"

#include <Heliogrim/TextureFormat.hpp>

using namespace hg::game::assets;
using namespace hg;

texture::IconTemplate::IconTemplate() :
    TextureAsset(
        texture::IconTemplate::unstable_auto_guid(),
        image::IconTemplate::unstable_auto_guid(),
        {},
        { 144ui32, 144ui32, 1ui32 },
        TextureFormat::eR8G8B8A8Srgb,
        1ui32,
        engine::gfx::TextureType::e2d
    ) { }
