#pragma once

#include "./stdafx.h"

namespace ember {
    using Uuid = uuid_t;

    inline void UuidGenerate(Uuid* uuid_) {
        #ifdef _WIN32
        CoCreateGuid(uuid_);
        #else
#pragma error("Not implemented uuid generator.")
        #endif
    }
}
