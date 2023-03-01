#include "GrassWild01Normal.hpp"
#include "../Images/GrassWild01Normal.hpp"

#include <Heliogrim/TextureFormat.hpp>

using namespace hg::game::assets::texture;
using namespace hg;

GrassWild01Normal::GrassWild01Normal() :
    TextureAsset(
        GrassWild01Normal::unstable_auto_guid(),
        game::assets::image::GrassWild01Normal::unstable_auto_guid(),
        {},
        {},
        TextureFormat::eR8G8B8Unorm,
        0,
        engine::gfx::TextureType::e2d
    ) { }
