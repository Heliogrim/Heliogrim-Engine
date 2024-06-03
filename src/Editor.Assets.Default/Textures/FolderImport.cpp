#include "FolderImport.hpp"
#include "../Images/FolderImport.hpp"

#include <Heliogrim/TextureFormat.hpp>

using namespace hg::game::assets;
using namespace hg;

texture::FolderImport::FolderImport() :
    TextureAsset(
        clone(texture::FolderImport::unstable_auto_guid()),
        image::FolderImport::unstable_auto_guid(),
        {},
        { 144u, 144u, 1u },
        TextureFormat::eR8G8B8A8Srgb,
        1uL,
        engine::gfx::TextureType::e2d
    ) {}
