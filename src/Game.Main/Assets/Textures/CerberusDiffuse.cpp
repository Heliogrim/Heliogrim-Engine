#include "CerberusDiffuse.hpp"
#include "../Images/CerberusDiffuse.hpp"

#include <Ember/TextureFormat.hpp>

#include "Game.Main/Assets/Images/CerberusDiffuse.hpp"

using namespace ember::game::assets;
using namespace ember;

texture::CerberusDiffuse::CerberusDiffuse() :
    TextureAsset(
        texture::CerberusDiffuse::unstable_auto_guid(),
        image::CerberusDiffuse::unstable_auto_guid(),
        {},
        { 8192ui32, 8192ui32, 1ui32 },
        TextureFormat::eR8G8B8A8Unorm,
        14ui32,
        engine::gfx::TextureType::e2d
    ) { }
