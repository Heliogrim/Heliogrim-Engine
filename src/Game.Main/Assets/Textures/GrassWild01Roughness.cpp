#include "GrassWild01Roughness.hpp"
#include "../Images/GrassWild01Roughness.hpp"

#include <Heliogrim/TextureFormat.hpp>

using namespace hg::game::assets::texture;
using namespace hg;

GrassWild01Roughness::GrassWild01Roughness() :
    TextureAsset(
        GrassWild01Roughness::unstable_auto_guid(),
        game::assets::image::GrassWild01Roughness::unstable_auto_guid(),
        {},
        {},
        TextureFormat::eR8G8B8Unorm,
        0,
        engine::gfx::TextureType::e2d
    ) { }
