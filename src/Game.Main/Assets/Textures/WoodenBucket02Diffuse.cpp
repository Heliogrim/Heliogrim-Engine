#include "WoodenBucket02Diffuse.hpp"
#include "../Images/WoodenBucket02Diffuse.hpp"

#include <Heliogrim/TextureFormat.hpp>

#include "Game.Main/Assets/Images/WoodenBucket02Diffuse.hpp"

using namespace hg::game::assets;
using namespace hg;

texture::WoodenBucket02Diffuse::WoodenBucket02Diffuse() :
    TextureAsset(
        texture::WoodenBucket02Diffuse::unstable_auto_guid(),
        image::WoodenBucket02Diffuse::unstable_auto_guid(),
        {},
        { 8192ui32, 8192ui32, 1ui32 },
        TextureFormat::eR8G8B8A8Unorm,
        14ui32,
        engine::gfx::TextureType::e2d
    ) { }
