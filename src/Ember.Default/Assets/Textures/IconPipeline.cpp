#include "IconPipeline.hpp"
#include "../Images/IconPipeline.hpp"

#include <Ember/TextureFormat.hpp>

using namespace ember::game::assets;
using namespace ember;

texture::IconPipeline::IconPipeline() :
    TextureAsset(
        texture::IconPipeline::auto_guid(),
        image::IconPipeline::auto_guid(),
        {},
        { 144ui32, 144ui32, 1ui32 },
        TextureFormat::eR8G8B8A8Srgb,
        1ui32,
        engine::gfx::TextureType::e2d
    ) { }
