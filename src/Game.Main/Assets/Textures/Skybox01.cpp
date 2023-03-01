#include "Skybox01.hpp"
#include "../Images/Skybox01.hpp"

#include <Heliogrim/TextureFormat.hpp>

using namespace hg::game::assets;
using namespace hg;

texture::Skybox01Diffuse::Skybox01Diffuse() :
    TextureAsset(
        texture::Skybox01Diffuse::unstable_auto_guid(),
        image::Skybox01Diffuse::unstable_auto_guid(),
        {},
        { 1024ui32, 1024ui32, 1ui32 },
        TextureFormat::eR16G16B16A16Sfloat,
        11ui32,
        engine::gfx::TextureType::eCube
    ) { }
