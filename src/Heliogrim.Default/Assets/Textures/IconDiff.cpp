#include "IconDiff.hpp"
#include "../Images/IconDiff.hpp"

#include <Heliogrim/TextureFormat.hpp>

using namespace hg::game::assets;
using namespace hg;

texture::IconDiff::IconDiff() :
    TextureAsset(
        texture::IconDiff::unstable_auto_guid(),
        image::IconDiff::unstable_auto_guid(),
        {},
        { 144ui32, 144ui32, 1ui32 },
        TextureFormat::eR8G8B8A8Srgb,
        1ui32,
        engine::gfx::TextureType::e2d
    ) { }
