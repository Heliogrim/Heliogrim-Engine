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
        { 1u, 1u, 1u },
        TextureFormat::eR16Sfloat,
        1uL,
        engine::gfx::TextureType::e2d
    ) { }
