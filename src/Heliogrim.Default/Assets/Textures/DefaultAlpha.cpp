#include "DefaultAlpha.hpp"
#include "../Images/DefaultAlpha.hpp"

#include <Heliogrim/TextureFormat.hpp>

using namespace hg::game::assets;
using namespace hg;

texture::DefaultAlpha::DefaultAlpha() :
    TextureAsset(
        clone(texture::DefaultAlpha::unstable_auto_guid()),
        image::DefaultAlpha::unstable_auto_guid(),
        {},
        { 1ui32, 1ui32, 1ui32 },
        TextureFormat::eR16Sfloat,
        1ui32,
        engine::gfx::TextureType::e2d
    ) {}
