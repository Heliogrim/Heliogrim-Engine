#pragma once

namespace ember {

    template <typename From_, typename To_, typename VoidType = void>
    struct void_cast {
        [[nodiscard]] To_ operator()(From_ value_) const noexcept {
            return *reinterpret_cast<To_*>(
                reinterpret_cast<VoidType*>(&value_)
            );
        }
    };

    template <typename From_, typename To_>
    struct void_cast<const From_&, const To_&, const void> { };

    template <typename From_, typename To_>
    struct void_cast<const From_, const To_, const void> { };

    template <typename From_, typename To_>
    struct void_cast<From_&, To_&> { };
}
