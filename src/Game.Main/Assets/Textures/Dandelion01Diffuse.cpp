#include "Dandelion01Diffuse.hpp"
#include "../Images/Dandelion01Diffuse.hpp"

#include <Ember/TextureFormat.hpp>

using namespace ember::game::assets;
using namespace ember;

texture::Dandelion01Diffuse::Dandelion01Diffuse() :
    TextureAsset(
        texture::Dandelion01Diffuse::auto_guid(),
        image::Dandelion01Diffuse::auto_guid(),
        {},
        { 8192ui32, 8192ui32, 1ui32 },
        TextureFormat::eR8G8B8A8Unorm,
        13ui32,
        engine::gfx::TextureType::e2d
    ) { }
