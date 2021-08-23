#pragma once
#include "Data.hpp"
#include "SoundLayout.hpp"

namespace ember::engine::assets {
    class Sound :
        public Data<SoundLayout> { };
}
