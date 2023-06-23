#pragma once

#include <concepts>

namespace hg {
    template <typename Type_>
    concept is_clonable = _STD false_type::value;

    [[nodiscard]] auto clone(const auto& val_) {
        return val_;
    }
}
