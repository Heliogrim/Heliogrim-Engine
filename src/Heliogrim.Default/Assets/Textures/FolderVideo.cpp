#include "FolderVideo.hpp"
#include "../Images/FolderVideo.hpp"

#include <Heliogrim/TextureFormat.hpp>

using namespace hg::game::assets;
using namespace hg;

texture::FolderVideo::FolderVideo() :
    TextureAsset(
        clone(texture::FolderVideo::unstable_auto_guid()),
        image::FolderVideo::unstable_auto_guid(),
        {},
        { 144u, 144u, 1u },
        TextureFormat::eR8G8B8A8Srgb,
        1uL,
        engine::gfx::TextureType::e2d
    ) {}
