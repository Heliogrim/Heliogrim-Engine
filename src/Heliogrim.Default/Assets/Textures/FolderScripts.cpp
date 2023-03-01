#include "FolderScripts.hpp"
#include "../Images/FolderScripts.hpp"

#include <Heliogrim/TextureFormat.hpp>

using namespace hg::game::assets;
using namespace hg;

texture::FolderScripts::FolderScripts() :
    TextureAsset(
        texture::FolderScripts::unstable_auto_guid(),
        image::FolderScripts::unstable_auto_guid(),
        {},
        { 144ui32, 144ui32, 1ui32 },
        TextureFormat::eR8G8B8A8Srgb,
        1ui32,
        engine::gfx::TextureType::e2d
    ) { }
