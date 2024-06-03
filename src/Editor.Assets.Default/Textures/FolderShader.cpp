#include "FolderShader.hpp"
#include "../Images/FolderShader.hpp"

#include <Heliogrim/TextureFormat.hpp>

using namespace hg::game::assets;
using namespace hg;

texture::FolderShader::FolderShader() :
    TextureAsset(
        clone(texture::FolderShader::unstable_auto_guid()),
        image::FolderShader::unstable_auto_guid(),
        {},
        { 144u, 144u, 1u },
        TextureFormat::eR8G8B8A8Srgb,
        1uL,
        engine::gfx::TextureType::e2d
    ) {}
