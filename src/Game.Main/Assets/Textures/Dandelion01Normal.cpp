#include "Dandelion01Normal.hpp"
#include "../Images/Dandelion01Normal.hpp"

#include <Heliogrim/TextureFormat.hpp>

using namespace hg::game::assets;
using namespace hg;

texture::Dandelion01Normal::Dandelion01Normal() :
    TextureAsset(
        texture::Dandelion01Normal::unstable_auto_guid(),
        image::Dandelion01Normal::unstable_auto_guid(),
        {},
        { 8192ui32, 8192ui32, 1ui32 },
        TextureFormat::eR32G32B32A32Sfloat,
        14ui32,
        engine::gfx::TextureType::e2d
    ) { }
