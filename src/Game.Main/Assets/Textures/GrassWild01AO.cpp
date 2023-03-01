#include "GrassWild01AO.hpp"
#include "../Images/GrassWild01AO.hpp"

#include <Heliogrim/TextureFormat.hpp>

using namespace hg::game::assets::texture;
using namespace hg;

GrassWild01AO::GrassWild01AO() :
    TextureAsset(
        GrassWild01AO::unstable_auto_guid(),
        game::assets::image::GrassWild01AO::unstable_auto_guid(),
        {},
        {},
        TextureFormat::eR8G8B8Unorm,
        0,
        engine::gfx::TextureType::e2d
    ) { }
