#pragma once

namespace hg {
    template <typename From_, typename To_, typename VoidType = void>
    struct void_cast {
        [[nodiscard]] constexpr To_ operator()(From_ value_) const noexcept {
            return *reinterpret_cast<std::remove_reference_t<To_>*>(
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

    template <typename Ty_, typename Ux_, typename Vy_ = void>
    [[nodiscard]] constexpr Ty_* _void_cast(Ux_* ptr_) noexcept {
        return reinterpret_cast<Ty_*>(reinterpret_cast<Vy_*>(ptr_));
    }

    template <typename Ty_, typename Ux_, typename Vy_ = void>
    [[nodiscard]] constexpr std::remove_pointer_t<Ty_>* _void_cast(
        std::remove_pointer_t<std::remove_reference_t<Ux_>>*&& ptr_
    ) noexcept {
        using ty = std::remove_pointer_t<Ty_>;
        using ux = std::remove_pointer_t<std::remove_reference_t<Ux_>>;
        return reinterpret_cast<ty*>(reinterpret_cast<Vy_*>(std::forward<ux*>(ptr_)));
    }
}
