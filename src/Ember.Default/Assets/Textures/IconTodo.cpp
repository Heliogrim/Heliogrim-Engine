#include "IconTodo.hpp"
#include "../Images/IconTodo.hpp"

#include <Ember/TextureFormat.hpp>

using namespace ember::game::assets;
using namespace ember;

texture::IconTodo::IconTodo() :
    TextureAsset(
        texture::IconTodo::auto_guid(),
        image::IconTodo::auto_guid(),
        {},
        { 144ui32, 144ui32, 1ui32 },
        TextureFormat::eR8G8B8A8Srgb,
        1ui32,
        engine::gfx::TextureType::e2d
    ) { }
