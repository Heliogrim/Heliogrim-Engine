#include "GrassWild01Displacement.hpp"
#include "../Images/GrassWild01Displacement.hpp"

#include <Heliogrim/TextureFormat.hpp>

using namespace hg::game::assets::texture;
using namespace hg;

GrassWild01Displacement::GrassWild01Displacement() :
    TextureAsset(
        GrassWild01Displacement::unstable_auto_guid(),
        game::assets::image::GrassWild01Displacement::unstable_auto_guid(),
        {},
        {},
        TextureFormat::eR8G8B8Unorm,
        0,
        engine::gfx::TextureType::e2d
    ) { }
