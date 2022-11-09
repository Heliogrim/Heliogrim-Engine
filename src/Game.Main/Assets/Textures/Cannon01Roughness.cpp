#include "Cannon01Roughness.hpp"
#include "../Images/Cannon01Roughness.hpp"

#include <Ember/TextureFormat.hpp>

#include "Game.Main/Assets/Images/Cannon01Roughness.hpp"

using namespace ember::game::assets;
using namespace ember;

texture::Cannon01Roughness::Cannon01Roughness() :
    TextureAsset(
        texture::Cannon01Roughness::unstable_auto_guid(),
        image::Cannon01Roughness::unstable_auto_guid(),
        {},
        { 8192ui32, 8192ui32, 1ui32 },
        TextureFormat::eR8Unorm,
        14ui32,
        engine::gfx::TextureType::e2d
    ) { }
