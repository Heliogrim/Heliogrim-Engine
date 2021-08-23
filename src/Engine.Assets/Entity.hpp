#pragma once
#include "Data.hpp"
#include "EntityLayout.hpp"

namespace ember::engine::assets {
    class Entity :
        public Data<EntityLayout> {};
}
