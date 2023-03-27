#include "DefaultMaterial2.hpp"
#include "../Textures/DefaultDiffuse.hpp"
#include "../Textures/DefaultAO.hpp"
#include "../Textures/DefaultMetalness.hpp"
#include "../Textures/DefaultNormal.hpp"
#include "../Textures/DefaultRoughness.hpp"

using namespace hg::game::assets::material;
using namespace hg;

DefaultMaterial2::DefaultMaterial2() :
    GfxMaterialAsset(
        DefaultMaterial2::auto_guid<DefaultMaterial2>(),
        asset_guid { 463735861, 57288, 18838, 6811336636808021432ui64 },
        asset_guid { 3154289946, 48722, 20360, 11494976344687249817ui64 },
        invalid_asset_guid,
        invalid_asset_guid,
        invalid_asset_guid,
        asset_guid { 250785504, 57013, 18368, 7845085242770483085ui64 },
        asset_guid { 3036373222, 58971, 20230, 7397490891087128467ui64 },
        asset_guid { 544899431, 37305, 19763, 15138426091184268470ui64 },
        invalid_asset_guid,
        invalid_asset_guid
    ) {}
