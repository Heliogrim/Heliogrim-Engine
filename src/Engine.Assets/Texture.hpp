#pragma once
#include "Data.hpp"
#include "TextureLayout.hpp"

namespace ember::engine::assets {
    class Texture :
        public Data<TextureLayout> { };
}
