#include "DefaultMetalness.hpp"
#include "../Images/DefaultMetalness.hpp"

#include <Heliogrim/TextureFormat.hpp>

using namespace hg::game::assets;
using namespace hg;

texture::DefaultMetalness::DefaultMetalness() :
    TextureAsset(
        clone(texture::DefaultMetalness::unstable_auto_guid()),
        image::DefaultMetalness::unstable_auto_guid(),
        {},
        { 1ui32, 1ui32, 1ui32 },
        TextureFormat::eR16Sfloat,
        1ui32,
        engine::gfx::TextureType::e2d
    ) {}
