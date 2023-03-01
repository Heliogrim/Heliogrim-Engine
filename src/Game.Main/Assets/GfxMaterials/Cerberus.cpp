#include "Cerberus.hpp"

#include "Game.Main/Assets/Textures/CerberusAO.hpp"
#include "Game.Main/Assets/Textures/CerberusDiffuse.hpp"
#include "Game.Main/Assets/Textures/CerberusMetalness.hpp"
#include "Game.Main/Assets/Textures/CerberusNormal.hpp"
#include "Game.Main/Assets/Textures/CerberusRoughness.hpp"

using namespace hg::game::assets::material;
using namespace hg;

Cerberus::Cerberus() :
    GfxMaterialAsset(
        Cerberus::unstable_auto_guid(),
        texture::CerberusDiffuse::unstable_auto_guid(),
        texture::CerberusAO::unstable_auto_guid(),
        invalid_asset_guid,
        invalid_asset_guid,
        invalid_asset_guid,
        texture::CerberusNormal::unstable_auto_guid(),
        texture::CerberusRoughness::unstable_auto_guid(),
        texture::CerberusMetalness::unstable_auto_guid(),
        invalid_asset_guid,
        invalid_asset_guid
    ) {
    setAssetName("Cerberus Material"sv);
}
