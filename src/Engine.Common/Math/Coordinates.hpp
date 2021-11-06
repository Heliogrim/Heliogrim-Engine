#pragma once

#include "../Wrapper.hpp"
#include "Vector.hpp"

namespace ember::math {

    inline constexpr static vec3 vec3_zero { 0, 0, 0 };

    inline constexpr static vec3 vec3_one { 1, 1, 1 };

    inline constexpr static vec3 vec3_up { 0, 1, 0 };

    inline constexpr static vec3 vec3_down { 0, -1, 0 };

    inline constexpr static vec3 vec3_left { -1, 0, 0 };

    inline constexpr static vec3 vec3_right { 1, 0, 0 };

    inline constexpr static vec3 vec3_forward { 0, 0, 1 };

    inline constexpr static vec3 vec3_backward { 0, 0, -1 };

    inline constexpr static vec3 vec3_pitch { 1, 0, 0 };

    inline constexpr static vec3 vec3_yaw { 0, 1, 0 };

    inline constexpr static vec3 vec3_roll { 0, 0, 1 };

    inline constexpr static vec2 vec2_zero { 0, 0 };

    inline constexpr static vec2 vec2_one { 0, 0 };

    inline constexpr static vec2 vec2_up { 0, 1 };

    inline constexpr static vec2 vec2_down { 0, -1 };

    inline constexpr static vec2 vec2_left { -1, 0 };

    inline constexpr static vec2 vec2_right { 1, 0 };

    namespace polar {

        inline constexpr static vec3 vec3_distance = { 1, 0, 0 };

        inline constexpr static vec3 vec3_azimuth = { 0, 1, 0 };

        inline constexpr static vec3 vec3_polar = { 0, 0, 1 };

        inline constexpr static vec2 vec2_distance = { 1, 0 };

        inline constexpr static vec2 vec2_polar = { 0, 1 };

        [[nodiscard]] vec3 from_cartesian(_In_ cref<vec3> cartesian_) noexcept;

        [[nodiscard]] vec3 from_cartesian(_In_ mref<vec3> cartesian_) noexcept;

        [[nodiscard]] vec2 from_cartesian(_In_ cref<vec2> cartesian_) noexcept;

        [[nodiscard]] vec2 from_cartesian(_In_ mref<vec2> cartesian_) noexcept;

        [[nodiscard]] vec3 to_cartesian(_In_ cref<vec3> polar_) noexcept;

        [[nodiscard]] vec3 to_cartesian(_In_ mref<vec3> polar_) noexcept;

        [[nodiscard]] vec2 to_cartesian(_In_ cref<vec2> polar_) noexcept;

        [[nodiscard]] vec2 to_cartesian(_In_ mref<vec2> polar_) noexcept;

    }

}
