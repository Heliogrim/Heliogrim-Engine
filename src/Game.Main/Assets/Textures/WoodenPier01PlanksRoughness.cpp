#include "WoodenPier01PlanksRoughness.hpp"
#include "../Images/WoodenPier01PlanksRoughness.hpp"

#include <Heliogrim/TextureFormat.hpp>

using namespace hg::game::assets;
using namespace hg;

texture::WoodenPier01PlanksRoughness::WoodenPier01PlanksRoughness() :
    TextureAsset(
        texture::WoodenPier01PlanksRoughness::unstable_auto_guid(),
        image::WoodenPier01PlanksRoughness::unstable_auto_guid(),
        {},
        { 8192ui32, 8192ui32, 1ui32 },
        TextureFormat::eR8Unorm,
        14ui32,
        engine::gfx::TextureType::e2d
    ) { }
