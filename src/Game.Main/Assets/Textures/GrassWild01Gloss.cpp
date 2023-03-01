#include "GrassWild01Gloss.hpp"
#include "../Images/GrassWild01Gloss.hpp"

#include <Heliogrim/TextureFormat.hpp>

using namespace hg::game::assets::texture;
using namespace hg;

GrassWild01Gloss::GrassWild01Gloss() :
    TextureAsset(
        GrassWild01Gloss::unstable_auto_guid(),
        game::assets::image::GrassWild01Gloss::unstable_auto_guid(),
        {},
        {},
        TextureFormat::eR8G8B8Unorm,
        0,
        engine::gfx::TextureType::e2d
    ) { }
