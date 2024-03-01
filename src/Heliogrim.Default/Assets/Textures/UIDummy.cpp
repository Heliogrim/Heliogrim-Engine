#include "UIDummy.hpp"
#include "../Images/UIDummy.hpp"

#include <Heliogrim/TextureFormat.hpp>

using namespace hg::game::assets;
using namespace hg;

texture::UIDummy::UIDummy() :
    TextureAsset(
        clone(texture::UIDummy::unstable_auto_guid()),
        image::UIDummy::unstable_auto_guid(),
        {},
        { 1u, 1u, 1u },
        TextureFormat::eR8G8B8A8Unorm,
        1uL,
        engine::gfx::TextureType::e2d
    ) {}
