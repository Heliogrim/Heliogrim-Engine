#include "Brand.hpp"
#include "../Images/Brand.hpp"

#include <Heliogrim/TextureFormat.hpp>

using namespace hg::game::assets;
using namespace hg;

texture::Brand::Brand() :
    TextureAsset(
        clone(texture::Brand::unstable_auto_guid()),
        image::Brand::unstable_auto_guid(),
        {},
        { 512u, 512u, 1u },
        TextureFormat::eR8G8B8A8Srgb,
        1uL,
        engine::gfx::TextureType::e2d
    ) {}
