#include "Cerberus.hpp"

#include "Game.Main/Assets/Textures/CerberusAO.hpp"
#include "Game.Main/Assets/Textures/CerberusDiffuse.hpp"
#include "Game.Main/Assets/Textures/CerberusMetalness.hpp"
#include "Game.Main/Assets/Textures/CerberusNormal.hpp"
#include "Game.Main/Assets/Textures/CerberusRoughness.hpp"

using namespace ember::game::assets::material;
using namespace ember;

Cerberus::Cerberus() :
    GfxMaterialAsset(
        Cerberus::auto_guid(),
        texture::CerberusDiffuse::auto_guid(),
        texture::CerberusAO::auto_guid(),
        invalid_asset_guid,
        invalid_asset_guid,
        invalid_asset_guid,
        texture::CerberusNormal::auto_guid(),
        texture::CerberusRoughness::auto_guid(),
        texture::CerberusMetalness::auto_guid(),
        invalid_asset_guid,
        invalid_asset_guid
    ) {}
