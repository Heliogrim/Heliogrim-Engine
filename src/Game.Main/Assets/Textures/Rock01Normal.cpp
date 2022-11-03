#include "Rock01Normal.hpp"
#include "../Images/Rock01Normal.hpp"

#include <Ember/TextureFormat.hpp>

using namespace ember::game::assets;
using namespace ember;

texture::Rock01Normal::Rock01Normal() :
    TextureAsset(
        texture::Rock01Normal::unstable_auto_guid(),
        image::Rock01Normal::unstable_auto_guid(),
        {},
        { 8192ui32, 8192ui32, 1ui32 },
        TextureFormat::eR8G8B8A8Unorm,
        14ui32,
        engine::gfx::TextureType::e2d
    ) { }
