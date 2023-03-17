#include "DefaultMaterial.hpp"
#include "../Textures/DefaultDiffuse.hpp"
#include "../Textures/DefaultAO.hpp"
#include "../Textures/DefaultMetalness.hpp"
#include "../Textures/DefaultNormal.hpp"
#include "../Textures/DefaultRoughness.hpp"

using namespace hg::game::assets::material;
using namespace hg;

DefaultMaterial::DefaultMaterial() :
    GfxMaterialAsset(
        DefaultMaterial::auto_guid<DefaultMaterial>(),
        asset_guid { 1513411454, 43702, 17570, 414804866245091486 },
        asset_guid { 4215950391, 3450, 20261, 18172857380330759046ui64 },
        invalid_asset_guid,
        invalid_asset_guid,
        invalid_asset_guid,
        asset_guid { 2957839326, 51539, 17441, 445097573227334828 },
        asset_guid { 3532173099, 19939, 16567, 8852593991083949459 },
        asset_guid { 2408835102, 717, 16804, 7791177201199205771ui64 },
        invalid_asset_guid,
        invalid_asset_guid
    ) {}
