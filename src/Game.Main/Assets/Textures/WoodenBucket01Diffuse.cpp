#include "WoodenBucket01Diffuse.hpp"
#include "../Images/WoodenBucket01Diffuse.hpp"

#include <Ember/TextureFormat.hpp>

#include "Game.Main/Assets/Images/WoodenPier01PlanksDiffuse.hpp"

using namespace ember::game::assets;
using namespace ember;

texture::WoodenBucket01Diffuse::WoodenBucket01Diffuse() :
    TextureAsset(
        texture::WoodenBucket01Diffuse::unstable_auto_guid(),
        image::WoodenBucket01Diffuse::unstable_auto_guid(),
        {},
        { 8192ui32, 8192ui32, 1ui32 },
        TextureFormat::eR8G8B8A8Unorm,
        14ui32,
        engine::gfx::TextureType::e2d
    ) { }
