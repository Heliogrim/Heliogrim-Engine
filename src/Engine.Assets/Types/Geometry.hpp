#pragma once
#include "Data.hpp"
#include "GeometryLayout.hpp"

namespace ember::engine::assets {

    class Geometry :
        public Data<GeometryLayout> { };
}
