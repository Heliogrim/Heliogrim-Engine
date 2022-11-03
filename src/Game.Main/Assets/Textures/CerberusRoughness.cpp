#include "CerberusRoughness.hpp"
#include "../Images/CerberusRoughness.hpp"

#include <Ember/TextureFormat.hpp>

#include "Game.Main/Assets/Images/CerberusRoughness.hpp"

using namespace ember::game::assets;
using namespace ember;

texture::CerberusRoughness::CerberusRoughness() :
    TextureAsset(
        texture::CerberusRoughness::unstable_auto_guid(),
        image::CerberusRoughness::unstable_auto_guid(),
        {},
        { 8192ui32, 8192ui32, 1ui32 },
        TextureFormat::eR8Unorm,
        14ui32,
        engine::gfx::TextureType::e2d
    ) { }
