#pragma once

#include "../stdafx.h"

namespace ember {
    class NotImplementedException :
        public std::logic_error {
    public:
        NotImplementedException() :
            std::logic_error("Function not yet implemented.") { }
    };
}
