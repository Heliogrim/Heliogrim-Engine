#include "IconDatabase.hpp"
#include "../Images/IconDatabase.hpp"

#include <Heliogrim/TextureFormat.hpp>

using namespace hg::game::assets;
using namespace hg;

texture::IconDatabase::IconDatabase() :
    TextureAsset(
        texture::IconDatabase::unstable_auto_guid(),
        image::IconDatabase::unstable_auto_guid(),
        {},
        { 144ui32, 144ui32, 1ui32 },
        TextureFormat::eR8G8B8A8Srgb,
        1ui32,
        engine::gfx::TextureType::e2d
    ) { }
