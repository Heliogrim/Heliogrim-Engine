#include "IconTree.hpp"
#include "../Images/IconTree.hpp"

#include <Ember/TextureFormat.hpp>

using namespace ember::game::assets;
using namespace ember;

texture::IconTree::IconTree() :
    TextureAsset(
        texture::IconTree::unstable_auto_guid(),
        image::IconTree::unstable_auto_guid(),
        {},
        { 144ui32, 144ui32, 1ui32 },
        TextureFormat::eR8G8B8A8Srgb,
        1ui32,
        engine::gfx::TextureType::e2d
    ) { }
