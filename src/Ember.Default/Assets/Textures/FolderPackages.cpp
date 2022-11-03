#include "FolderPackages.hpp"
#include "../Images/FolderPackages.hpp"

#include <Ember/TextureFormat.hpp>

using namespace ember::game::assets;
using namespace ember;

texture::FolderPackages::FolderPackages() :
    TextureAsset(
        texture::FolderPackages::unstable_auto_guid(),
        image::FolderPackages::unstable_auto_guid(),
        {},
        { 144ui32, 144ui32, 1ui32 },
        TextureFormat::eR8G8B8A8Srgb,
        1ui32,
        engine::gfx::TextureType::e2d
    ) { }
