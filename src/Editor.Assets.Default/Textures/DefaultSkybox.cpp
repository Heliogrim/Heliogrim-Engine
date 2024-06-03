#include "DefaultSkybox.hpp"
#include "../Images/DefaultSkybox.hpp"

#include <Heliogrim/TextureFormat.hpp>

using namespace hg::game::assets;
using namespace hg;

texture::DefaultSkybox::DefaultSkybox() :
    TextureAsset(
        clone(texture::DefaultSkybox::unstable_auto_guid()),
        image::DefaultSkybox::unstable_auto_guid(),
        {},
        { 2048u, 2048u, 1u },
        TextureFormat::eR16G16B16A16Sfloat,
        12uL,
        engine::gfx::TextureType::eCube
    ) {}
