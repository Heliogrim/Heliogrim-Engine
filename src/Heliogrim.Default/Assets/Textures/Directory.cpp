#include "Directory.hpp"
#include "../Images/Directory.hpp"

#include <Heliogrim/TextureFormat.hpp>

using namespace hg::game::assets;
using namespace hg;

texture::Directory::Directory() :
    TextureAsset(
        clone(texture::Directory::unstable_auto_guid()),
        image::Directory::unstable_auto_guid(),
        {},
        { 144u, 144u, 1u },
        TextureFormat::eR8G8B8A8Srgb,
        1uL,
        engine::gfx::TextureType::e2d
    ) {}
