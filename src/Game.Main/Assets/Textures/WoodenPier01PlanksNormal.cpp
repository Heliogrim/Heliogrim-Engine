#include "WoodenPier01PlanksNormal.hpp"
#include "../Images/WoodenPier01PlanksNormal.hpp"

#include <Ember/TextureFormat.hpp>

using namespace ember::game::assets;
using namespace ember;

texture::WoodenPier01PlanksNormal::WoodenPier01PlanksNormal() :
    TextureAsset(
        texture::WoodenPier01PlanksNormal::auto_guid(),
        image::WoodenPier01PlanksNormal::auto_guid(),
        {},
        { 8192ui32, 8192ui32, 1ui32 },
        TextureFormat::eR8G8B8A8Unorm,
        14ui32,
        engine::gfx::TextureType::e2d
    ) { }
