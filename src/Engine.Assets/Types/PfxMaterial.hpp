#pragma once
#include "Material.hpp"
#include "PfxMaterialLayout.hpp"

namespace ember::engine::assets {

    class PfxMaterial :
        public Material<PfxMaterialLayout> { };
}
