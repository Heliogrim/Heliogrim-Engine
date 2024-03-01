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
        { 1u, 1u, 1u },
        TextureFormat::eR16Sfloat,
        1uL,
        engine::gfx::TextureType::e2d
    ) {}
