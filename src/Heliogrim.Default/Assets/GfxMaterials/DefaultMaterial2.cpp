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
        // 2464635825-23784-20215-18413211476781495477
        asset_guid { 2464635825, 23784, 20215, 18413211476781495477ui64 },
        // 2876130772-41426-20079-11952607629922539648
        asset_guid { 2876130772, 41426, 20079, 11952607629922539648ui64 },
        invalid_asset_guid,
        invalid_asset_guid,
        invalid_asset_guid,
        // 258031152-27438-18562-17364142873412929425
        asset_guid { 258031152, 27438, 18562, 17364142873412929425ui64 },
        // 4291491352-24741-20021-5349468694551058864
        asset_guid { 4291491352, 24741, 20021, 5349468694551058864ui64 },
        // 851278636-5713-17652-1520732405900800676
        asset_guid { 851278636, 5713, 17652, 1520732405900800676ui64 },
        invalid_asset_guid,
        invalid_asset_guid
    ) {}
