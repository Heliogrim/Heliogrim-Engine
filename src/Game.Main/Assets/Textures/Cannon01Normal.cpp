#include "Cannon01Normal.hpp"
#include "../Images/Cannon01Normal.hpp"

#include <Heliogrim/TextureFormat.hpp>

#include "Game.Main/Assets/Images/Cannon01Normal.hpp"

using namespace hg::game::assets;
using namespace hg;

texture::Cannon01Normal::Cannon01Normal() :
    TextureAsset(
        texture::Cannon01Normal::unstable_auto_guid(),
        image::Cannon01Normal::unstable_auto_guid(),
        {},
        { 8192ui32, 8192ui32, 1ui32 },
        TextureFormat::eR8G8B8A8Unorm,
        14ui32,
        engine::gfx::TextureType::e2d
    ) { }
