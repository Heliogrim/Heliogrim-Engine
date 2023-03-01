#include "GrassWild01Albedo.hpp"
#include "../Images/GrassWild01Albedo.hpp"

#include <Heliogrim/TextureFormat.hpp>

using namespace hg::game::assets::texture;
using namespace hg;

GrassWild01Albedo::GrassWild01Albedo() :
    TextureAsset(
        GrassWild01Albedo::unstable_auto_guid(),
        game::assets::image::GrassWild01Albedo::unstable_auto_guid(),
        {},
        {},
        TextureFormat::eR8G8B8Unorm,
        0,
        engine::gfx::TextureType::e2d
    ) { }
