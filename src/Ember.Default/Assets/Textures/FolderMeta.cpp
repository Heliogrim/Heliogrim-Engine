#include "FolderMeta.hpp"
#include "../Images/FolderMeta.hpp"

#include <Ember/TextureFormat.hpp>

using namespace ember::game::assets;
using namespace ember;

texture::FolderMeta::FolderMeta() :
    TextureAsset(
        texture::FolderMeta::unstable_auto_guid(),
        image::FolderMeta::unstable_auto_guid(),
        {},
        { 144ui32, 144ui32, 1ui32 },
        TextureFormat::eR8G8B8A8Srgb,
        1ui32,
        engine::gfx::TextureType::e2d
    ) { }
