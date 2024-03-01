#include "FolderEnvironment.hpp"
#include "../Images/FolderEnvironment.hpp"

#include <Heliogrim/TextureFormat.hpp>

using namespace hg::game::assets;
using namespace hg;

texture::FolderEnvironment::FolderEnvironment() :
    TextureAsset(
        clone(texture::FolderEnvironment::unstable_auto_guid()),
        image::FolderEnvironment::unstable_auto_guid(),
        {},
        { 144u, 144u, 1u },
        TextureFormat::eR8G8B8A8Srgb,
        1uL,
        engine::gfx::TextureType::e2d
    ) {}
