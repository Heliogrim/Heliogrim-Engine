#include "DefaultNormal.hpp"
#include "../Images/DefaultNormal.hpp"

#include <Heliogrim/TextureFormat.hpp>

using namespace hg::game::assets;
using namespace hg;

texture::DefaultNormal::DefaultNormal() :
    TextureAsset(
        clone(texture::DefaultNormal::unstable_auto_guid()),
        image::DefaultNormal::unstable_auto_guid(),
        {},
        { 1u, 1u, 1u },
        TextureFormat::eR32G32B32A32Sfloat,
        1uL,
        engine::gfx::TextureType::e2d
    ) {}
