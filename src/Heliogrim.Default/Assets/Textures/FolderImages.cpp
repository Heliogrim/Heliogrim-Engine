#include "FolderImages.hpp"
#include "../Images/FolderImages.hpp"

#include <Heliogrim/TextureFormat.hpp>

using namespace hg::game::assets;
using namespace hg;

texture::FolderImages::FolderImages() :
    TextureAsset(
        clone(texture::FolderImages::unstable_auto_guid()),
        image::FolderImages::unstable_auto_guid(),
        {},
        { 144u, 144u, 1u },
        TextureFormat::eR8G8B8A8Srgb,
        1uL,
        engine::gfx::TextureType::e2d
    ) {}
