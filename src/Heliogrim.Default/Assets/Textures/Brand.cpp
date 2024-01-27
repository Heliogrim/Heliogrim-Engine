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
        { 512ui32, 512ui32, 1ui32 },
        TextureFormat::eR8G8B8A8Srgb,
        1ui32,
        engine::gfx::TextureType::e2d
    ) {}
