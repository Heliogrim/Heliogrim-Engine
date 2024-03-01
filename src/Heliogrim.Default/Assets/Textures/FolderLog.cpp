#include "FolderLog.hpp"
#include "../Images/FolderLog.hpp"

#include <Heliogrim/TextureFormat.hpp>

using namespace hg::game::assets;
using namespace hg;

texture::FolderLog::FolderLog() :
    TextureAsset(
        clone(texture::FolderLog::unstable_auto_guid()),
        image::FolderLog::unstable_auto_guid(),
        {},
        { 144u, 144u, 1u },
        TextureFormat::eR8G8B8A8Srgb,
        1uL,
        engine::gfx::TextureType::e2d
    ) {}
