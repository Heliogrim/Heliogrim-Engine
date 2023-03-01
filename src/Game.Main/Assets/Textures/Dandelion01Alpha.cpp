#include "Dandelion01Alpha.hpp"
#include "../Images/Dandelion01Alpha.hpp"

#include <Heliogrim/TextureFormat.hpp>

using namespace hg::game::assets;
using namespace hg;

texture::Dandelion01Alpha::Dandelion01Alpha() :
    TextureAsset(
        texture::Dandelion01Alpha::unstable_auto_guid(),
        image::Dandelion01Alpha::unstable_auto_guid(),
        {},
        { 8192ui32, 8192ui32, 1ui32 },
        TextureFormat::eR16Sfloat,
        14ui32,
        engine::gfx::TextureType::e2d
    ) { }
