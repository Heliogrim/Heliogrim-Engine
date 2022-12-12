#include "FolderAnimation.hpp"
#include "../Images/FolderAnimation.hpp"

#include <Ember/TextureFormat.hpp>

using namespace ember::game::assets;
using namespace ember;

texture::FolderAnimation::FolderAnimation() :
    TextureAsset(
        texture::FolderAnimation::unstable_auto_guid(),
        image::FolderAnimation::unstable_auto_guid(),
        {},
        { 144ui32, 144ui32, 1ui32 },
        TextureFormat::eR8G8B8A8Srgb,
        1ui32,
        engine::gfx::TextureType::e2d
    ) { }
