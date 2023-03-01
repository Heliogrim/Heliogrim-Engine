#include "GrassWild01Specular.hpp"
#include "../Images/GrassWild01Specular.hpp"

#include <Heliogrim/TextureFormat.hpp>

using namespace hg::game::assets::texture;
using namespace hg;

GrassWild01Specular::GrassWild01Specular() :
    TextureAsset(
        GrassWild01Specular::unstable_auto_guid(),
        game::assets::image::GrassWild01Specular::unstable_auto_guid(),
        {},
        {},
        TextureFormat::eR8G8B8Unorm,
        0,
        engine::gfx::TextureType::e2d
    ) { }
