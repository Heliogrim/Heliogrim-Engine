#include "DefaultAO.hpp"
#include "../Images/DefaultAO.hpp"

#include <Heliogrim/TextureFormat.hpp>

using namespace hg::game::assets;
using namespace hg;

texture::DefaultAO::DefaultAO() :
    TextureAsset(
        clone(texture::DefaultAO::unstable_auto_guid()),
        image::DefaultAO::unstable_auto_guid(),
        {},
        { 1u, 1u, 1u },
        TextureFormat::eR16Sfloat,
        1uL,
        engine::gfx::TextureType::e2d
    ) {}
