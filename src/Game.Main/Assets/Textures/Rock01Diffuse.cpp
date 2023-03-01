#include "Rock01Diffuse.hpp"
#include "../Images/Rock01Diffuse.hpp"

#include <Heliogrim/TextureFormat.hpp>

using namespace hg::game::assets;
using namespace hg;

texture::Rock01Diffuse::Rock01Diffuse() :
    TextureAsset(
        texture::Rock01Diffuse::unstable_auto_guid(),
        image::Rock01Diffuse::unstable_auto_guid(),
        {},
        { 8192ui32, 8192ui32, 1ui32 },
        TextureFormat::eR8G8B8A8Unorm,
        14ui32,
        engine::gfx::TextureType::e2d
    ) { }
