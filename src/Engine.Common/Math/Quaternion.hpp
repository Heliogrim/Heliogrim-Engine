#pragma once
#include <cstdint>
#include <cmath>
//#include <complex>

#include "../GLM.hpp"
#include <glm/gtc/quaternion.hpp>

#include "MathDefaultDefine.hpp"
#include "Vector.hpp"

namespace hg::math {
    /**
     * \brief 
     * \tparam Ly 
     * \tparam Ry 
     * \param x 
     * \param eps 
     * \return 
     */
    template <typename Ly, typename Ry>
    inline bool is_scalar_zero(Ly x, Ry eps = 0) {
        typedef typename std::common_type<Ly, Ry>::type Ty;
        Ty xx = static_cast<Ty>(x);
        Ty ee = static_cast<Ty>(eps);
        return std::abs(xx) <= ee;
    }

    /**
     * A quaternion t.
     *
     * @author Julius
     * @date 16.01.2020
     *
     * @tparam T Generic type parameter.
     */
    template <typename T>
    class quaternion_t {
    public:
        /** Defines an alias representing type of the value */
        typedef T value_type;
        /** Defines an alias representing the type */
        typedef quaternion_t<value_type> type;
        /** Defines an alias representing type of the reference */
        typedef type& reference_type;
        /** Defines an alias representing type of the constant reference */
        typedef const type& const_reference_type;

        /**
         * 
         */
        static_assert(
            std::is_same<T, bool>()
            || std::is_same<T, int>()
            || std::is_same<T, long>()
            || std::is_same<T, long long>()
            || std::is_same<T, float>()
            || std::is_same<T, double>()
            || std::is_same<T, long double>(),
            "Invalid scalar type for Quaternion"
        );

        /**
         * Default constructor
         *
         * @author Julius
         * @date 16.01.2020
         */
        constexpr quaternion_t() noexcept :
            _quat() {}

        /**
         * Constructor
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @param  x_ The x coordinate.
         * @param  y_ (Optional) The y coordinate.
         * @param  z_ (Optional) The z coordinate.
         * @param  w_ (Optional) The w.
         */
        constexpr quaternion_t(value_type x_, value_type y_ = 0, value_type z_ = 0, value_type w_ = 0) noexcept :
            _quat(w_, x_, y_, z_) {}

        /**
         * \brief 
         * \tparam Ty
         * \param x_ 
         * \param y_ 
         * \param z_ 
         * \param w_ 
         */
        template <typename Ty, typename = _STD enable_if_t<_STD is_convertible<Ty, T>::value>>
        quaternion_t(Ty x_ = 0, Ty y_ = 0, Ty z_ = 0, Ty w_ = 0) :
            _quat(static_cast<T>(w_), static_cast<T>(x_), static_cast<T>(y_), static_cast<T>(z_)) {}

        /**
         * Destructor
         *
         * @author Julius
         * @date 16.01.2020
         */
        ~quaternion_t() = default;

        [[nodiscard]] vec3_t<value_type> __forward() const {
            return vec3_t<value_type> {
                2 * (_quat.x * _quat.y + _quat.w * _quat.y),
                2 * (_quat.y * _quat.z + _quat.w * _quat.x),
                1 - 2 * (_quat.x * _quat.x + _quat.y * _quat.y),
            };
        }

        [[nodiscard]] vec3_t<value_type> __up() const {
            return vec3_t<value_type> {
                2 * (_quat.x * _quat.y + _quat.w * _quat.z),
                1 - 2 * (_quat.x * _quat.x + _quat.z * _quat.z),
                2 * (_quat.y * _quat.z + _quat.w * _quat.x),
            };
        }

        [[nodiscard]] vec3_t<value_type> _left() const {
            return vec3_t<value_type> {
                1 - 2 * (_quat.y * _quat.y + _quat.z * _quat.z),
                2 * (_quat.x * _quat.y + _quat.w * _quat.z),
                2 * (_quat.x * _quat.z + _quat.w * _quat.y),
            };
        }

        /**
         * Gets the pitch
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @returns A value_type.
         */
        value_type pitch() const {
            return glm::pitch(_quat);
        }

        /**
         * Gets the yaw in radians
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @returns A value_type.
         */
        value_type yaw() const {
            return glm::yaw(_quat);
        }

        /**
         * Gets the roll in radians
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @returns A value_type.
         */
        value_type roll() const {
            return glm::roll(_quat);
        }

        /**
         * Gets the euler angles in radians
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @returns A vec3_t&lt;value_type&gt;
         */
        vec3_t<value_type> euler() const {
            return vec3_t<value_type>(
                glm::pitch(_quat),
                glm::yaw(_quat),
                glm::roll(_quat)
            );
        }

        /**
         * Euler angles to quaternion
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @param  angles_ The angles.
         *
         * @returns A type.
         */
        static type euler(typename vec3_t<value_type>::const_reference_type angles_) {
            const auto pitch = glm::angleAxis(angles_.x, glm::vec3(1, 0, 0));
            const auto yaw = glm::angleAxis(angles_.y, glm::vec3(0, 1, 0));
            const auto roll = glm::angleAxis(angles_.z, glm::vec3(0, 0, 1));

            type result = type();
            result._quat = pitch * yaw * roll;

            return result;
        }

        /**
         *
         */
        constexpr static type make_identity() {
            return type { 0, 0, 0, 1 };
        }

        /**
         * Addition assignment operator
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @param  other_ The other.
         *
         * @returns The result of the operation.
         */
        type operator +=(const_reference_type other_) {
            _quat += other_._quat;
            return *this;
        }

        /**
         * Subtraction assignment operator
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @param  other_ The other.
         *
         * @returns The result of the operation.
         */
        type operator -=(const_reference_type other_) {
            _quat -= other_._quat;
            return *this;
        }

        /**
         * Multiplication assignment operator
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @param  other_ The other.
         *
         * @returns The result of the operation.
         */
        type operator *=(const_reference_type other_) {
            _quat *= other_._quat;
            return *this;
        }

        /**
         * Division assignment operator
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @param  other_ The other.
         *
         * @returns The result of the operation.
         */
        type operator /=(const_reference_type other_) {
            _quat /= other_._quat;
            return *this;
        }

        /**
         * Multiplication operator
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @param  other_ The other.
         *
         * @returns The result of the operation.
         */
        type operator *(const_reference_type other_) const {
            type result = *this;
            return result *= other_;
        }

        /**
         * Division operator
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @param  other_ The other.
         *
         * @returns The result of the operation.
         */
        type operator /(const_reference_type other_) const {
            type result = *this;
            return result /= other_;
        }

        // union {
        // 	struct {
        // 		value_type a, b, c, d;
        // 	};
        // };
        glm::tquat<value_type, glm::highp> _quat;
    };

    typedef quaternion_t<double_t> dquaternion;
    typedef quaternion_t<float_t> fquaternion;
    typedef quaternion_t<int32_t> iquaternion;
    typedef quaternion_t<uint32_t> uiquaternion;

    #ifdef MATH_DEFAULT_SINGLE_PRECISION
    using quaternion = fquaternion;
    #endif
    #ifdef MATH_DEFAULT_DOUBLE_PRECISION
	using quaternion = dquaternion;
    #endif
}
