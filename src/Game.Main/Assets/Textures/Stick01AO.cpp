#include "Stick01AO.hpp"
#include "../Images/Stick01AO.hpp"

#include <Heliogrim/TextureFormat.hpp>

using namespace hg::game::assets;
using namespace hg;

texture::Stick01AO::Stick01AO() :
    TextureAsset(
        texture::Stick01AO::unstable_auto_guid(),
        image::Stick01AO::unstable_auto_guid(),
        {},
        { 8192ui32, 8192ui32, 1ui32 },
        TextureFormat::eR16Sfloat,
        14ui32,
        engine::gfx::TextureType::e2d
    ) { }
