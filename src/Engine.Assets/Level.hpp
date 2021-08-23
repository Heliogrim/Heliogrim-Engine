#pragma once
#include "Data.hpp"
#include "LevelLayout.hpp"

namespace ember::engine::assets {
    class Level :
        public Data<LevelLayout> { };
}
