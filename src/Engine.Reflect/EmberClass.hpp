#pragma once
#include "EmberStruct.hpp"

namespace ember {

    class EmberClass :
        public EmberStruct {
    public:
        using this_type = EmberClass;
        using underlying_type = EmberStruct;
    };
}
