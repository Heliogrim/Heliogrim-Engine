#include "IconCMake.hpp"
#include "../Images/IconCMake.hpp"

#include <Heliogrim/TextureFormat.hpp>

using namespace hg::game::assets;
using namespace hg;

texture::IconCMake::IconCMake() :
    TextureAsset(
        texture::IconCMake::unstable_auto_guid(),
        image::IconCMake::unstable_auto_guid(),
        {},
        { 144ui32, 144ui32, 1ui32 },
        TextureFormat::eR8G8B8A8Srgb,
        1ui32,
        engine::gfx::TextureType::e2d
    ) { }
