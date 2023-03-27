#pragma once

#include "Guid.hpp"
#include "String.hpp"

namespace hg {
    extern string encodeGuid4228(cref<Guid> guid_);

    extern Guid decodeGuid4228(string_view str_);
}
