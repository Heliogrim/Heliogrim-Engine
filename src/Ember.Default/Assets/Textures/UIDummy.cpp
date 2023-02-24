#include "UIDummy.hpp"
#include "../Images/UIDummy.hpp"

#include <Ember/TextureFormat.hpp>

using namespace ember::game::assets;
using namespace ember;

texture::UIDummy::UIDummy() :
    TextureAsset(
        texture::UIDummy::unstable_auto_guid(),
        image::UIDummy::unstable_auto_guid(),
        {},
        { 1ui32, 1ui32, 1ui32 },
        TextureFormat::eR8G8B8A8Unorm,
        1ui32,
        engine::gfx::TextureType::e2d
    ) { }
