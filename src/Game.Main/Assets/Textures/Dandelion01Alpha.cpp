#include "Dandelion01Alpha.hpp"
#include "../Images/Dandelion01Alpha.hpp"

#include <Ember/TextureFormat.hpp>

using namespace ember::game::assets;
using namespace ember;

texture::Dandelion01Alpha::Dandelion01Alpha() :
    TextureAsset(
        texture::Dandelion01Alpha::auto_guid(),
        image::Dandelion01Alpha::auto_guid(),
        {},
        { 8192ui32, 8192ui32, 1ui32 },
        TextureFormat::eR16Sfloat,
        14ui32,
        engine::gfx::TextureType::e2d
    ) { }
