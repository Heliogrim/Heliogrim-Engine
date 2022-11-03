#include "WoodenPier01Planks.hpp"

#include "../Textures/DefaultRoughness.hpp"
#include "../Textures/DefaultAO.hpp"
#include "../Textures/DefaultNormal.hpp"
#include "../Textures/DefaultDiffuse.hpp"
#include "../Textures/WoodenPier01PlanksDiffuse.hpp"
#include "../Textures/WoodenPier01PlanksNormal.hpp"
#include "../Textures/DefaultMetalness.hpp"
#include "../Textures/WoodenPier01PlanksRoughness.hpp"

using namespace ember::game::assets::material;
using namespace ember;

WoodenPier01Planks::WoodenPier01Planks() :
    GfxMaterialAsset(
        WoodenPier01Planks::unstable_auto_guid(),
        texture::WoodenPier01PlanksDiffuse::unstable_auto_guid(),
        texture::DefaultAO::unstable_auto_guid(),
        invalid_asset_guid,
        invalid_asset_guid,
        invalid_asset_guid,
        texture::WoodenPier01PlanksNormal::unstable_auto_guid(),
        texture::WoodenPier01PlanksRoughness::unstable_auto_guid(),
        texture::DefaultMetalness::unstable_auto_guid(),
        invalid_asset_guid,
        invalid_asset_guid
    ) {}
