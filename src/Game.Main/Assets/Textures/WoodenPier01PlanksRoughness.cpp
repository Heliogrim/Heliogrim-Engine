#include "WoodenPier01PlanksRoughness.hpp"
#include "../Images/WoodenPier01PlanksRoughness.hpp"

#include <Ember/TextureFormat.hpp>

using namespace ember::game::assets;
using namespace ember;

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
