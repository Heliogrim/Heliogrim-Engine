#pragma once

#include <concepts>

#include "Resource.hpp"

namespace ember::engine::res {

    template <class Type_>
    concept IsResource = std::derived_from<Type_, Resource>;

    template <IsResource Type_>
    using PartialIdentity = Type_;

}
