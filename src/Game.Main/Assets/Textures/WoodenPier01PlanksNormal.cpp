#include "WoodenPier01PlanksNormal.hpp"
#include "../Images/WoodenPier01PlanksNormal.hpp"

#include <Heliogrim/TextureFormat.hpp>

using namespace hg::game::assets;
using namespace hg;

texture::WoodenPier01PlanksNormal::WoodenPier01PlanksNormal() :
    TextureAsset(
        texture::WoodenPier01PlanksNormal::unstable_auto_guid(),
        image::WoodenPier01PlanksNormal::unstable_auto_guid(),
        {},
        { 8192ui32, 8192ui32, 1ui32 },
        TextureFormat::eR8G8B8A8Unorm,
        14ui32,
        engine::gfx::TextureType::e2d
    ) { }
