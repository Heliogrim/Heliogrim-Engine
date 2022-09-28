#include "CerberusMetalness.hpp"
#include "../Images/CerberusMetalness.hpp"

#include <Ember/TextureFormat.hpp>

#include "Game.Main/Assets/Images/CerberusMetalness.hpp"

using namespace ember::game::assets;
using namespace ember;

texture::CerberusMetalness::CerberusMetalness() :
    TextureAsset(
        texture::CerberusMetalness::auto_guid(),
        image::CerberusMetalness::auto_guid(),
        {},
        { 8192ui32, 8192ui32, 1ui32 },
        TextureFormat::eR8Unorm,
        14ui32,
        engine::gfx::TextureType::e2d
    ) { }
