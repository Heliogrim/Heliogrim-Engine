#include "IconCpp.hpp"
#include "../Images/IconCpp.hpp"

#include <Heliogrim/TextureFormat.hpp>

using namespace hg::game::assets;
using namespace hg;

texture::IconCpp::IconCpp() :
    TextureAsset(
        texture::IconCpp::unstable_auto_guid(),
        image::IconCpp::unstable_auto_guid(),
        {},
        { 144ui32, 144ui32, 1ui32 },
        TextureFormat::eR8G8B8A8Srgb,
        1ui32,
        engine::gfx::TextureType::e2d
    ) { }
