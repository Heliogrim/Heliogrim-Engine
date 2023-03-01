#include "DefaultRoughness.hpp"
#include "../Images/DefaultRoughness.hpp"

#include <Heliogrim/TextureFormat.hpp>

using namespace hg::game::assets;
using namespace hg;

texture::DefaultRoughness::DefaultRoughness() :
    TextureAsset(
        texture::DefaultRoughness::unstable_auto_guid(),
        image::DefaultRoughness::unstable_auto_guid(),
        {},
        { 1ui32, 1ui32, 1ui32 },
        TextureFormat::eR16Sfloat,
        1ui32,
        engine::gfx::TextureType::e2d
    ) { }
