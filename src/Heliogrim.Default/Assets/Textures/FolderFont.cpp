#include "FolderFont.hpp"
#include "../Images/FolderFont.hpp"

#include <Heliogrim/TextureFormat.hpp>

using namespace hg::game::assets;
using namespace hg;

texture::FolderFont::FolderFont() :
    TextureAsset(
        clone(texture::FolderFont::unstable_auto_guid()),
        image::FolderFont::unstable_auto_guid(),
        {},
        { 144u, 144u, 1u },
        TextureFormat::eR8G8B8A8Srgb,
        1uL,
        engine::gfx::TextureType::e2d
    ) {}
