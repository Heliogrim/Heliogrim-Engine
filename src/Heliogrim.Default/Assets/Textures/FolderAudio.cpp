#include "FolderAudio.hpp"
#include "../Images/FolderAudio.hpp"

#include <Heliogrim/TextureFormat.hpp>

using namespace hg::game::assets;
using namespace hg;

texture::FolderAudio::FolderAudio() :
    TextureAsset(
        clone(texture::FolderAudio::unstable_auto_guid()),
        image::FolderAudio::unstable_auto_guid(),
        {},
        { 144u, 144u, 1u },
        TextureFormat::eR8G8B8A8Srgb,
        1uL,
        engine::gfx::TextureType::e2d
    ) {}
