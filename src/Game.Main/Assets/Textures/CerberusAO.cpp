#include "CerberusAO.hpp"
#include "../Images/CerberusAO.hpp"

#include <Heliogrim/TextureFormat.hpp>

#include "Game.Main/Assets/Images/CerberusAO.hpp"

using namespace hg::game::assets;
using namespace hg;

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
