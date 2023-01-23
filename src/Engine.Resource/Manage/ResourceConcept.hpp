#pragma once

#include <concepts>

#include "__fwd.hpp"
#include "Resource.hpp"

namespace ember::engine::resource {
    template <class Type_>
    concept IsResource = _STD derived_from<Type_, ResourceBase>;

    template <typename ValueType_>
    concept IsResourceValue = true;

    template <IsResource Type_>
    using PartialIdentity = Type_;
}
