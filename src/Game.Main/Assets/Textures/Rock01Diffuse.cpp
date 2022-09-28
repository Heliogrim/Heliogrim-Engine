#include "Rock01Diffuse.hpp"
#include "../Images/Rock01Diffuse.hpp"

#include <Ember/TextureFormat.hpp>

using namespace ember::game::assets;
using namespace ember;

texture::Rock01Diffuse::Rock01Diffuse() :
    TextureAsset(
        texture::Rock01Diffuse::auto_guid(),
        image::Rock01Diffuse::auto_guid(),
        {},
        { 4096ui32, 4096ui32, 1ui32 },
        TextureFormat::eR8G8B8A8Unorm,
        12ui32,
        engine::gfx::TextureType::e2d
    ) { }
