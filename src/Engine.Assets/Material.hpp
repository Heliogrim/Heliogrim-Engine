#pragma once
#include "Data.hpp"
#include "MaterialLayout.hpp"

namespace ember::engine::assets {
    class Material :
        public Data<MaterialLayout> { };
}
