#include "DefaultRoughness.hpp"
#include "../Images/DefaultRoughness.hpp"

#include <Engine.Pedantic/Clone/Clone.hpp>
#include <Heliogrim/TextureFormat.hpp>

using namespace hg::game::assets;
using namespace hg;

texture::DefaultRoughness::DefaultRoughness() :
    TextureAsset(
        clone(texture::DefaultRoughness::unstable_auto_guid()),
        image::DefaultRoughness::unstable_auto_guid(),
        {},
        { 1u, 1u, 1u },
        TextureFormat::eR16Sfloat,
        1uL,
        engine::gfx::TextureType::e2d
    ) { }
