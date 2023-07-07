#include "Cerberus.hpp"

using namespace hg::game::assets::material;
using namespace hg;

Cerberus::Cerberus() :
    GfxMaterialAsset(Cerberus::unstable_auto_guid()) {
    setAssetName("Cerberus Material"sv);
}
