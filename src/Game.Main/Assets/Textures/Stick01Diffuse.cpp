#include "Stick01Diffuse.hpp"
#include "../Images/Stick01Diffuse.hpp"

#include <Heliogrim/TextureFormat.hpp>

using namespace hg::game::assets;
using namespace hg;

texture::Stick01Diffuse::Stick01Diffuse() :
    TextureAsset(
        texture::Stick01Diffuse::unstable_auto_guid(),
        image::Stick01Diffuse::unstable_auto_guid(),
        {},
        { 8192ui32, 8192ui32, 1ui32 },
        TextureFormat::eR8G8B8A8Unorm,
        14ui32,
        engine::gfx::TextureType::e2d
    ) { }
