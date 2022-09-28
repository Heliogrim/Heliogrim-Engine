#include "Stick01Diffuse.hpp"
#include "../Images/Stick01Diffuse.hpp"

#include <Ember/TextureFormat.hpp>

using namespace ember::game::assets;
using namespace ember;

texture::Stick01Diffuse::Stick01Diffuse() :
    TextureAsset(
        texture::Stick01Diffuse::auto_guid(),
        image::Stick01Diffuse::auto_guid(),
        {},
        { 8192ui32, 8192ui32, 1ui32 },
        TextureFormat::eR8G8B8A8Unorm,
        14ui32,
        engine::gfx::TextureType::e2d
    ) { }
