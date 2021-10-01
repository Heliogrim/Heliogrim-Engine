#pragma once
#include "Material.hpp"
#include "SfxMaterialLayout.hpp"

namespace ember::engine::assets {

    class SfxMaterial :
        public Material<SfxMaterialLayout> { };
}
