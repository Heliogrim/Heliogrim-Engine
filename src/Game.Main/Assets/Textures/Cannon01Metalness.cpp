#include "Cannon01Metalness.hpp"
#include "../Images/Cannon01Metalness.hpp"

#include <Ember/TextureFormat.hpp>

#include "Game.Main/Assets/Images/Cannon01Metalness.hpp"

using namespace ember::game::assets;
using namespace ember;

texture::Cannon01Metalness::Cannon01Metalness() :
    TextureAsset(
        texture::Cannon01Metalness::unstable_auto_guid(),
        image::Cannon01Metalness::unstable_auto_guid(),
        {},
        { 8192ui32, 8192ui32, 1ui32 },
        TextureFormat::eR8Unorm,
        14ui32,
        engine::gfx::TextureType::e2d
    ) { }
