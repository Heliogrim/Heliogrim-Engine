#include "CerberusAO.hpp"
#include "../Images/CerberusAO.hpp"

#include <Ember/TextureFormat.hpp>

#include "Game.Main/Assets/Images/CerberusAO.hpp"

using namespace ember::game::assets;
using namespace ember;

texture::CerberusAO::CerberusAO() :
    TextureAsset(
        texture::CerberusAO::unstable_auto_guid(),
        image::CerberusAO::unstable_auto_guid(),
        {},
        { 8192ui32, 8192ui32, 1ui32 },
        TextureFormat::eR8Unorm,
        14ui32,
        engine::gfx::TextureType::e2d
    ) { }
