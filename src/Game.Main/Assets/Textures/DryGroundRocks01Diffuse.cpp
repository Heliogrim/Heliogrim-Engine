#include "DryGroundRocks01Diffuse.hpp"
#include "../Images/DryGroundRocks01Diffuse.hpp"

#include <Ember/TextureFormat.hpp>

using namespace ember::game::assets;
using namespace ember;

texture::DryGroundRocks01Diffuse::DryGroundRocks01Diffuse() :
    TextureAsset(
        texture::DryGroundRocks01Diffuse::auto_guid(),
        image::DryGroundRocks01Diffuse::auto_guid(),
        {},
        { 8192ui32, 8192ui32, 1ui32 },
        TextureFormat::eR8G8B8A8Unorm,
        14ui32,
        engine::gfx::TextureType::e2d
    ) { }
