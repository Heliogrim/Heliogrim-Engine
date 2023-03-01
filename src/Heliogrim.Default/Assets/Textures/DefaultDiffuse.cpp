#include "DefaultDiffuse.hpp"
#include "../Images/DefaultDiffuse.hpp"

#include <Heliogrim/TextureFormat.hpp>

using namespace hg::game::assets;
using namespace hg;

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
