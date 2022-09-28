#include "WoodenBucket02Diffuse.hpp"
#include "../Images/WoodenBucket02Diffuse.hpp"

#include <Ember/TextureFormat.hpp>

#include "Game.Main/Assets/Images/WoodenBucket02Diffuse.hpp"

using namespace ember::game::assets;
using namespace ember;

texture::WoodenBucket02Diffuse::WoodenBucket02Diffuse() :
    TextureAsset(
        texture::WoodenBucket02Diffuse::auto_guid(),
        image::WoodenBucket02Diffuse::auto_guid(),
        {},
        { 8192ui32, 8192ui32, 1ui32 },
        TextureFormat::eR8G8B8A8Unorm,
        14ui32,
        engine::gfx::TextureType::e2d
    ) { }
