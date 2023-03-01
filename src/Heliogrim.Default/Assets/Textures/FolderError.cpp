#include "FolderError.hpp"
#include "../Images/FolderError.hpp"

#include <Heliogrim/TextureFormat.hpp>

using namespace hg::game::assets;
using namespace hg;

texture::FolderError::FolderError() :
    TextureAsset(
        texture::FolderError::unstable_auto_guid(),
        image::FolderError::unstable_auto_guid(),
        {},
        { 144ui32, 144ui32, 1ui32 },
        TextureFormat::eR8G8B8A8Srgb,
        1ui32,
        engine::gfx::TextureType::e2d
    ) { }
