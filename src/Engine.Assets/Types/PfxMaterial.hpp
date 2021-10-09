#pragma once
#include "Material.hpp"
#include "PfxMaterialLayout.hpp"

namespace ember::engine::assets {

    class PfxMaterial :
        public Material<PfxMaterialLayout> {
    public:
        inline static const asset_type_id type_id { "PfxMaterial"_typeId };
    };
}
