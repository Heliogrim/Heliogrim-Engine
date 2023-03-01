#include "Stick01Normal.hpp"
#include "../Images/Stick01Normal.hpp"

#include <Heliogrim/TextureFormat.hpp>

using namespace hg::game::assets;
using namespace hg;

texture::Stick01Normal::Stick01Normal() :
    TextureAsset(
        texture::Stick01Normal::unstable_auto_guid(),
        image::Stick01Normal::unstable_auto_guid(),
        {},
        { 8192ui32, 8192ui32, 1ui32 },
        TextureFormat::eR32G32B32A32Sfloat,
        14ui32,
        engine::gfx::TextureType::e2d
    ) { }
