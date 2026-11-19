#pragma once

#include "Resource.hpp"
#include "__fwd.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import std;
#endif

namespace hg::engine::resource {
    template <class Type_>
    concept IsResource = std::derived_from<Type_, ResourceBase>;

    template <typename ValueType_>
    concept IsResourceValue = true;

    template <IsResource Type_>
    using PartialIdentity = Type_;
}
