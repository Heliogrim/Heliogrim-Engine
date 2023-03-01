#include "DefaultAO.hpp"
#include "../Images/DefaultAO.hpp"

#include <Heliogrim/TextureFormat.hpp>

using namespace hg::game::assets;
using namespace hg;

texture::DefaultAO::DefaultAO() :
    TextureAsset(
        texture::DefaultAO::unstable_auto_guid(),
        image::DefaultAO::unstable_auto_guid(),
        {},
        { 1ui32, 1ui32, 1ui32 },
        TextureFormat::eR16Sfloat,
        1ui32,
        engine::gfx::TextureType::e2d
    ) { }
