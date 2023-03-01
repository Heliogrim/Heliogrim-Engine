#include "GrassWild01Cavity.hpp"
#include "../Images/GrassWild01Cavity.hpp"

#include "Heliogrim/TextureFormat.hpp"

using namespace hg::game::assets::texture;
using namespace hg;

GrassWild01Cavity::GrassWild01Cavity() :
    TextureAsset(
        GrassWild01Cavity::unstable_auto_guid(),
        game::assets::image::GrassWild01Cavity::unstable_auto_guid(),
        {},
        {},
        TextureFormat::eR8G8B8Unorm,
        0,
        engine::gfx::TextureType::e2d
    ) { }
