#include "DefaultDiffuse.hpp"
#include "../Images/DefaultDiffuse.hpp"

#include <Heliogrim/TextureFormat.hpp>

using namespace hg::game::assets;
using namespace hg;

texture::DefaultDiffuse::DefaultDiffuse() :
    TextureAsset(
        texture::DefaultDiffuse::auto_guid(),
        image::DefaultDiffuse::auto_guid(),
        {},
        { 1u, 1u, 1u },
        TextureFormat::eR8G8B8A8Unorm,
        1uL,
        engine::gfx::TextureType::e2d
    ) { }
