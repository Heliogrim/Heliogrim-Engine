#include "UIDummy.hpp"
#include "../Images/UIDummy.hpp"

#include <Heliogrim/TextureFormat.hpp>

using namespace hg::game::assets;
using namespace hg;

texture::UIDummy::UIDummy() :
    TextureAsset(
        texture::UIDummy::unstable_auto_guid(),
        image::UIDummy::unstable_auto_guid(),
        {},
        { 1ui32, 1ui32, 1ui32 },
        TextureFormat::eR8G8B8A8Unorm,
        1ui32,
        engine::gfx::TextureType::e2d
    ) {}
