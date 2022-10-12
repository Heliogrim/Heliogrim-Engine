#include "DefaultDiffuse.hpp"
#include "../Images/DefaultDiffuse.hpp"

#include <Ember/TextureFormat.hpp>

using namespace ember::game::assets;
using namespace ember;

texture::DefaultDiffuse::DefaultDiffuse() :
    TextureAsset(
        texture::DefaultDiffuse::auto_guid(),
        image::DefaultDiffuse::auto_guid(),
        {},
        { 1ui32, 1ui32, 1ui32 },
        TextureFormat::eR8G8B8A8Unorm,
        1ui32,
        engine::gfx::TextureType::e2d
    ) { }
