#include "IconArchitecture.hpp"
#include "../Images/IconArchitecture.hpp"

#include <Heliogrim/TextureFormat.hpp>

using namespace hg::game::assets;
using namespace hg;

texture::IconArchitecture::IconArchitecture() :
    TextureAsset(
        texture::IconArchitecture::unstable_auto_guid(),
        image::IconArchitecture::unstable_auto_guid(),
        {},
        { 144ui32, 144ui32, 1ui32 },
        TextureFormat::eR8G8B8A8Srgb,
        1ui32,
        engine::gfx::TextureType::e2d
    ) { }
