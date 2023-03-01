#include "CerberusMetalness.hpp"
#include "../Images/CerberusMetalness.hpp"

#include <Heliogrim/TextureFormat.hpp>

#include "Game.Main/Assets/Images/CerberusMetalness.hpp"

using namespace hg::game::assets;
using namespace hg;

texture::CerberusMetalness::CerberusMetalness() :
    TextureAsset(
        texture::CerberusMetalness::unstable_auto_guid(),
        image::CerberusMetalness::unstable_auto_guid(),
        {},
        { 8192ui32, 8192ui32, 1ui32 },
        TextureFormat::eR8Unorm,
        14ui32,
        engine::gfx::TextureType::e2d
    ) { }
