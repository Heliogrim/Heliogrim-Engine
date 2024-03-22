#pragma once
#include "../Wrapper.hpp"
#include "Vector.hpp"
#include "Quaternion.hpp"

namespace hg::math {
    struct Rotator final :
        protected fvec3 {
    public:
        using this_type = Rotator;
        using math_type = fvec3;

        using value_type = fvec3::value_type;

    public:
        Rotator() noexcept;

        Rotator(const value_type pitch_, const value_type yaw_, const value_type roll_) noexcept;

        explicit Rotator(mref<quaternion> quatern_) noexcept;

        Rotator(mref<this_type> other_) noexcept;

        Rotator(cref<this_type> other_) noexcept;

        ~Rotator() noexcept = default;

    public:
        ref<this_type> operator=(mref<this_type> other_) noexcept;

        ref<this_type> operator=(cref<this_type> other_) noexcept;

    public:
        [[nodiscard]] value_type pitch() const noexcept;

        ref<this_type> setPitch(auto&& pitch_) noexcept {
            x = std::forward<decltype(pitch_)>(pitch_);
            return *this;
        }

        [[nodiscard]] value_type yaw() const noexcept;

        ref<this_type> setYaw(auto&& yaw_) noexcept {
            y = std::forward<decltype(yaw_)>(yaw_);
            return *this;
        }

        [[nodiscard]] value_type roll() const noexcept;

        ref<this_type> setRoll(auto&& roll_) noexcept {
            z = std::forward<decltype(roll_)>(roll_);
            return *this;
        }

    public:
        [[nodiscard]] math_type inversed(cref<math_type> vector_) const noexcept;

        ref<math_type> inverse(ref<math_type> vector_) const noexcept;

        [[nodiscard]] math_type rotated(cref<math_type> vector_) const noexcept;

        ref<math_type> rotate(ref<math_type> vector_) const noexcept;

    public:
        [[nodiscard]] this_type inversedRotator() const noexcept;

        ref<this_type> inverseRotator() noexcept;

        [[nodiscard]] value_type normalizeAxis(float angle_) const noexcept;

        [[nodiscard]] this_type normalizedRotator() const noexcept;

        ref<this_type> normalizeRotator() noexcept;

        [[nodiscard]] value_type denormalizeAxis(float angle_) const noexcept;

        [[nodiscard]] this_type denormalizedRotator() const noexcept;

        ref<this_type> denormalizeRotator() noexcept;

    public:
        [[nodiscard]] static this_type combine(cref<this_type> left_, cref<this_type> right_) noexcept;

        [[nodiscard]] static this_type diff(cref<this_type> from_, cref<this_type> to_) noexcept;

        [[nodiscard]] static this_type outerToInner(cref<this_type> subject_, cref<this_type> outer_) noexcept;

        [[nodiscard]] static this_type innerToOuter(cref<this_type> subject_, cref<this_type> inner_) noexcept;

    public:
        [[nodiscard]] this_type operator-(cref<this_type> other_) const noexcept;

        [[nodiscard]] this_type operator+(cref<this_type> other_) const noexcept;

        [[nodiscard]] this_type operator*(value_type scale_) const noexcept;

        ref<this_type> operator-=(cref<this_type> other_) noexcept;

        ref<this_type> operator+=(cref<this_type> other_) noexcept;

        ref<this_type> operator*=(value_type scale_) noexcept;

    public:
        [[nodiscard]] bool operator==(cref<this_type> other_) const noexcept;

        [[nodiscard]] bool operator!=(cref<this_type> other_) const noexcept;

    public:
        [[nodiscard]] fvec3 euler() const noexcept;

        [[nodiscard]] static Rotator fromEuler(cref<fvec3> euler_) noexcept;

        [[nodiscard]] static Rotator fromEulerRaw(cref<fvec3> euler_) noexcept;

    public:
        [[nodiscard]] quaternion into() const noexcept;
    };
}
