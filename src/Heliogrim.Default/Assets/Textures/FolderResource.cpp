#include "FolderResource.hpp"
#include "../Images/FolderResource.hpp"

#include <Heliogrim/TextureFormat.hpp>

using namespace hg::game::assets;
using namespace hg;

texture::FolderResource::FolderResource() :
    TextureAsset(
        clone(texture::FolderResource::unstable_auto_guid()),
        image::FolderResource::unstable_auto_guid(),
        {},
        { 144u, 144u, 1u },
        TextureFormat::eR8G8B8A8Srgb,
        1uL,
        engine::gfx::TextureType::e2d
    ) {}
