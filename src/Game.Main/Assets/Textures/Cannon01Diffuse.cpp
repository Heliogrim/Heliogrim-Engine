#include "Cannon01Diffuse.hpp"
#include "../Images/Cannon01Diffuse.hpp"

#include <Ember/TextureFormat.hpp>

#include "Game.Main/Assets/Images/Cannon01Diffuse.hpp"

using namespace ember::game::assets;
using namespace ember;

texture::Cannon01Diffuse::Cannon01Diffuse() :
    TextureAsset(
        texture::Cannon01Diffuse::unstable_auto_guid(),
        image::Cannon01Diffuse::unstable_auto_guid(),
        {},
        { 8192ui32, 8192ui32, 1ui32 },
        TextureFormat::eR8G8B8A8Unorm,
        14ui32,
        engine::gfx::TextureType::e2d
    ) { }
