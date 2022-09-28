#include "ForestGround01Diffuse.hpp"
#include "../Images/ForestGround01Diffuse.hpp"

#include <Ember/TextureFormat.hpp>

using namespace ember::game::assets;
using namespace ember;

texture::ForestGround01Diffuse::ForestGround01Diffuse() :
    TextureAsset(
        texture::ForestGround01Diffuse::auto_guid(),
        image::ForestGround01Diffuse::auto_guid(),
        {},
        { 8192ui32, 8192ui32, 1ui32 },
        TextureFormat::eR8G8B8A8Unorm,
        /*4ui32*/13ui32,
        engine::gfx::TextureType::e2d
    ) { }
