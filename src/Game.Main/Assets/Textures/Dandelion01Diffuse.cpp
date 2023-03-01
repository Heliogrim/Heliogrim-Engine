#include "Dandelion01Diffuse.hpp"
#include "../Images/Dandelion01Diffuse.hpp"

#include <Heliogrim/TextureFormat.hpp>

using namespace hg::game::assets;
using namespace hg;

texture::Dandelion01Diffuse::Dandelion01Diffuse() :
    TextureAsset(
        texture::Dandelion01Diffuse::unstable_auto_guid(),
        image::Dandelion01Diffuse::unstable_auto_guid(),
        {},
        { 8192ui32, 8192ui32, 1ui32 },
        TextureFormat::eR8G8B8A8Unorm,
        14ui32,
        engine::gfx::TextureType::e2d
    ) { }
