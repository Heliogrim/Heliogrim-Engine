#include "IconGit.hpp"
#include "../Images/IconGit.hpp"

#include <Heliogrim/TextureFormat.hpp>

using namespace hg::game::assets;
using namespace hg;

texture::IconGit::IconGit() :
    TextureAsset(
        texture::IconGit::unstable_auto_guid(),
        image::IconGit::unstable_auto_guid(),
        {},
        { 144ui32, 144ui32, 1ui32 },
        TextureFormat::eR8G8B8A8Srgb,
        1ui32,
        engine::gfx::TextureType::e2d
    ) { }
