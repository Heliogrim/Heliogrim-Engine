#pragma once
#include "Material.hpp"
#include "SfxMaterialLayout.hpp"

namespace ember::engine::assets {

    class SfxMaterial :
        public Material<SfxMaterialLayout> {
    public:
        inline static const asset_type_id type_id { "SfxMaterial"_typeId };
    };
}
