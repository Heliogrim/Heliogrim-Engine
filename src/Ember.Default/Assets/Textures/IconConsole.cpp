#include "IconConsole.hpp"
#include "../Images/IconConsole.hpp"

#include <Ember/TextureFormat.hpp>

using namespace ember::game::assets;
using namespace ember;

texture::IconConsole::IconConsole() :
    TextureAsset(
        texture::IconConsole::unstable_auto_guid(),
        image::IconConsole::unstable_auto_guid(),
        {},
        { 144ui32, 144ui32, 1ui32 },
        TextureFormat::eR8G8B8A8Srgb,
        1ui32,
        engine::gfx::TextureType::e2d
    ) { }
