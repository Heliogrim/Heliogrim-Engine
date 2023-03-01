#include "CerberusNormal.hpp"
#include "../Images/CerberusNormal.hpp"

#include <Heliogrim/TextureFormat.hpp>

#include "Game.Main/Assets/Images/CerberusNormal.hpp"

using namespace hg::game::assets;
using namespace hg;

texture::CerberusNormal::CerberusNormal() :
    TextureAsset(
        texture::CerberusNormal::unstable_auto_guid(),
        image::CerberusNormal::unstable_auto_guid(),
        {},
        { 8192ui32, 8192ui32, 1ui32 },
        TextureFormat::eR8G8B8A8Unorm,
        14ui32,
        engine::gfx::TextureType::e2d
    ) { }
