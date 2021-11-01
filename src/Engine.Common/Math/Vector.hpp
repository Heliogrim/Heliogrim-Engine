#pragma once
#include <cstdint>
#include <cmath>
#include <limits>

#include "../__macro.hpp"
#include "../Types.hpp"
#include "MathDefaultDefine.hpp"
#include "Hash.hpp"

/** . */
namespace ember::math {
    /* Class Forward */

    /**
     * A vector 2 t.
     *
     * @author Julius
     * @date 16.01.2020
     *
     * @tparam T Generic type parameter.
     */
    template <typename T>
    class vec2_t;

    /**
     * A vector 3 t.
     *
     * @author Julius
     * @date 16.01.2020
     *
     * @tparam T Generic type parameter.
     */
    template <typename T>
    class vec3_t;

    /**
     * A vector 4 t.
     *
     * @author Julius
     * @date 16.01.2020
     *
     * @tparam T Generic type parameter.
     */
    template <typename T>
    class vec4_t;

    /* !Class Forward */

    /**
     * A vector 2 t.
     *
     * @author Julius
     * @date 16.01.2020
     *
     * @tparam T Generic type parameter.
     */
    template <typename T>
    class vec2_t {
    public:
        /** Defines an alias representing type of the value */
        typedef T value_type;
        /** Defines an alias representing the type */
        typedef vec2_t<value_type> type;
        /** Defines an alias representing type of the reference */
        typedef type& reference_type;
        /** Defines an alias representing type of the constant reference */
        typedef const type& const_reference_type;

        /**
         * Default constructor
         *
         * @author Julius
         * @date 16.01.2020
         */
        vec2_t() :
            x(static_cast<value_type>(0)),
            y(static_cast<value_type>(0)) { }

        /**
         * Constructor
         *
         * @author Julius
         * @date 27.03.2021
         *
         * @param  value_ The x and y coordinate.
         */
        vec2_t(const value_type& value_) :
            x(value_),
            y(value_) {}

        /**
         * Constructor
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @param  x_ The x coordinate.
         * @param  y_ The y coordinate.
         */
        vec2_t(const value_type& x_, const value_type& y_) :
            x(x_),
            y(y_) { }

        /**
         * Constructor
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @param  xyz_ The xyz.
         */
        vec2_t(const vec3_t<T>& xyz_) :
            x(xyz_.x),
            y(xyz_.y) { }

        /**
         * Constructor
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @param  xyzw_ The xyzw.
         */
        vec2_t(const vec4_t<T>& xyzw_) :
            x(xyzw_.x),
            y(xyzw_.y) { }

        /**
         * Sets an x coordinate
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @param  x_ The x coordinate.
         *
         * @returns A vec2&lt;T&gt;
         */
        type setX(const value_type& x_) {
            x = x_;
            return *this;
        }

        /**
         * Sets a y coordinate
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @param  y_ The y coordinate.
         *
         * @returns A reference to a vec2&lt;T&gt;
         */
        type setY(const value_type& y_) {
            y = y_;
            return *this;
        }

        /**
         * Gets the length
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @returns A T.
         */
        value_type length() const {
            return static_cast<value_type>(std::sqrt(x * x + y * y));
        }

        /**
         * Dots the given other
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @param  other_ The other.
         *
         * @returns A T.
         */
        value_type dot(const_reference_type other_) const {
            const type tmp((*this) * other_);
            return tmp.x + tmp.y;
        }

        /**
         * Distances the given other
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @param  other_ The other.
         *
         * @returns A mathGenericType.
         */
        [[nodiscard]] mathGenericType distance(const_reference_type other_) const {
            return static_cast<mathGenericType>(std::sqrt(
                (x - other_.x) * (x - other_.x) + (y - other_.y) * (y - other_.y)));
        }

        /**
         * Scales the given scale
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @param  scale_ The scale.
         *
         * @returns A vec2&lt;T&gt;
         */
        type scale(const value_type& scale_) {
            x *= scale_;
            y *= scale_;
            return *this;
        }

        /**
         * Gets the normalize
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @returns A vec2&lt;T&gt;
         */
        type normalize() {
            const value_type l = length();
            scale(1 / l);
            return *this;
        }

        /**
         * Multiplication assignment operator
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @param  scalar_ The scalar.
         *
         * @returns The result of the operation.
         */
        type operator*=(const value_type& scalar_) {
            return scale(scalar_);
        }

        /**
         * Division assignment operator
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @param  scalar_ The scalar.
         *
         * @returns The result of the operation.
         */
        type operator/=(const value_type& scalar_) {
            return scale(1 / scalar_);
        }

        /**
         * Addition operator
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @param  other_ The other.
         *
         * @returns The result of the operation.
         */
        type operator+(const_reference_type other_) const {
            return type(x + other_.x, y + other_.y);
        }

        /**
         * Subtraction operator
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @param  other_ The other.
         *
         * @returns The result of the operation.
         */
        type operator-(const_reference_type other_) const {
            return type(x - other_.x, y - other_.y);
        }

        /**
         * Negation operator
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @returns The result of the operation.
         */
        type operator-() const {
            return type(-x, -y);
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
        type operator*(const_reference_type other_) const {
            return type(x * other_.x, y * other_.y);
        }

        /**
         * Multiplication operator
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @param  scalar_ The scalar.
         *
         * @returns The result of the operation.
         */
        type operator*(const value_type& scalar_) const {
            return type(x * scalar_, y * scalar_);
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
        type operator/(const_reference_type other_) const {
            return type(x / other_.x, y / other_.y);
        }

        /**
         * Division operator
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @param  scalar_ The scalar.
         *
         * @returns The result of the operation.
         */
        type operator/(const value_type& scalar_) const {
            return type(x / scalar_, y / scalar_);
        }

        /**
         * Equality operator
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @param  other_ The other.
         *
         * @returns True if the parameters are considered equivalent.
         */
        bool operator==(const_reference_type other_) const {
            return (x == other_.x && y == other_.y);
        }

        /**
         * Inequality operator
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @param  other_ The other.
         *
         * @returns True if the parameters are not considered equivalent.
         */
        bool operator!=(const_reference_type other_) const {
            return !(x == other_.x && y == other_.y);
        }

        /**
         * Array indexer operator
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @param  index_ Zero-based index of the.
         *
         * @returns The indexed value.
         */
        value_type operator[](const size_t index_) const {
            return (&x)[index_];
        }

        /**
         * Array indexer operator
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @param  index_ Zero-based index of the.
         *
         * @returns The indexed value.
         */
        value_type& operator[](const size_t index_) {
            return (&x)[index_];
        }

        /**
         * An unnamed.
         *
         * @author Julius
         * @date 16.01.2020
         */
        union {

            /**
             * An unnamed.
             *
             * @author Julius
             * @date 16.01.2020
             */
            struct {
                T x, y;
            };

            /**
             * An unnamed.
             *
             * @author Julius
             * @date 16.01.2020
             */
            struct {
                T s, t;
            };

            /**
             * An unnamed.
             *
             * @author Julius
             * @date 16.01.2020
             */
            struct {
                T u, v;
            };

            /**
             * An unnamed.
             *
             * @author Julius
             * @date 07.02.2020
             */
            struct {
                T min, max;
            };
        };
    };

    /**
     * A vector 3 t.
     *
     * @author Julius
     * @date 16.01.2020
     *
     * @tparam T Generic type parameter.
     */
    template <typename T>
    class vec3_t {
    public:
        /** Defines an alias representing type of the value */
        typedef T value_type;
        /** Defines an alias representing the type */
        typedef vec3_t<value_type> type;
        /** Defines an alias representing type of the reference */
        typedef type& reference_type;
        /** Defines an alias representing type of the constant reference */
        typedef const type& const_reference_type;

        /**
         * Default constructor
         *
         * @author Julius
         * @date 16.01.2020
         */
        constexpr vec3_t() noexcept :
            x(0),
            y(0),
            z(0) { }

        /**
         * Constructor
         *
         * @author Julius
         * @date 27.03.2021
         *
         * @param  value_ The x, y and z coordinate.
         */
        vec3_t(const value_type& value_) noexcept :
            x(value_),
            y(value_),
            z(value_) {}

        /**
         * Constructor
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @param  x_ The x coordinate.
         * @param  y_ The y coordinate.
         * @param  z_ The z coordinate.
         */
        vec3_t(const value_type& x_, const value_type& y_, const value_type& z_) :
            x(x_),
            y(y_),
            z(z_) { }

        /**
         * Constructor
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @param  xy_ The xy.
         * @param  z_  The z coordinate.
         */
        vec3_t(typename vec2_t<value_type>::const_reference_type xy_, const value_type& z_) :
            x(xy_.x),
            y(xy_.y),
            z(z_) { }

        /**
         * Constructor
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @param  xyzw_ The xyzw.
         */
        vec3_t(const vec4_t<value_type>& xyzw_) :
            x(xyzw_.x),
            y(xyzw_.y),
            z(xyzw_.z) { }

        /**
         * Sets an x coordinate
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @param  x_ The x coordinate.
         *
         * @returns A vec3&lt;T&gt;
         */
        type setX(const value_type& x_) {
            x = x_;
            return *this;
        }

        /**
         * Sets a y coordinate
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @param  y_ The y coordinate.
         *
         * @returns A reference to a vec3&lt;T&gt;
         */
        type setY(const value_type& y_) {
            y = y_;
            return *this;
        }

        /**
         * Sets a z coordinate
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @param  z_ The z coordinate.
         *
         * @returns A reference to a vec3&lt;T&gt;
         */
        type setZ(const value_type& z_) {
            z = z_;
            return *this;
        }

        /**
         * Gets the length
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @returns A T.
         */
        value_type length() const {
            return static_cast<value_type>(std::sqrt(x * x + y * y + z * z));
        }

        /**
         * Gets the square length
         *
         * @author Julius
         * @date 27.03.2021
         *
         * @returns A value_type.
         */
        value_type sqrLength() const {
            return static_cast<value_type>(x * x + y * y + z * z);
        }

        /**
         * Dots the given other
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @param  other_ The other.
         *
         * @returns A T.
         */
        value_type dot(const_reference_type other_) const {
            const type tmp { *this * other_ };
            return tmp.x + tmp.y + tmp.z;
        }

        /**
         * Distances the given other
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @param  other_ The other.
         *
         * @returns A mathGenericType.
         */
        [[nodiscard]] mathGenericType distance(const_reference_type other_) const {
            return static_cast<mathGenericType>(std::sqrt(
                (x - other_.x) * (x - other_.x) + (y - other_.y) * (y - other_.y) + (z - other_.z) * (z - other_.z)));
        }

        /**
         * Scales the given scale
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @param  scale_ The scale.
         *
         * @returns A vec3&lt;T&gt;
         */
        type scale(const value_type& scale_) {
            x *= scale_;
            y *= scale_;
            z *= scale_;
            return *this;
        }

        /**
         * Gets the normalize
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @returns A vec3&lt;T&gt;
         */
        type normalize() {
            DEBUG_STATIC_ASSERT(std::numeric_limits<value_type>::is_iec559, "'normalize' accepts only floating point")

            const value_type l = length();
            scale(1 / l);
            return *this;
        }

        /**
         * Gets the normalized
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @returns A vec3_t&lt;T&gt;
         */
        type normalized() const {
            DEBUG_STATIC_ASSERT(std::numeric_limits<value_type>::is_iec559, "'normalized' accepts only floating point")

            const value_type fac = (1 / length());
            return type(x * fac, y * fac, z * fac);
        }

        /**
         * Gets the cross
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @param  other_ The other.
         *
         * @returns A reference to a vec3_t&lt;T&gt;
         */
        type cross(const_reference_type other_) {
            DEBUG_STATIC_ASSERT(std::numeric_limits<value_type>::is_iec559, "'cross' accepts only floating point")

            const value_type x = this->x;
            const value_type y = this->y;
            const value_type z = this->z;

            this->x = (y * other_.z - other_.y * z);
            this->y = (z * other_.x - other_.z * x);
            this->z = (x * other_.y - other_.y * x);

            return *this;
        }

        /**
         * Cross the given other
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @param  other_ The other.
         *
         * @returns A vec3_t&lt;T&gt;
         */
        type crossed(const_reference_type other_) const {
            DEBUG_STATIC_ASSERT(std::numeric_limits<value_type>::is_iec559, "'crossed' accepts only floating point")

            return type(
                y * other_.z - other_.y * z,
                z * other_.x - other_.z * x,
                x * other_.y - other_.x * y
            );
        }

        /**
         * Check zero vector
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @returns whether vector is zero.
         */
        [[nodiscard]] bool zero() const {
            return x == 0 && y == 0 && z == 0;
        }

        /**
         * Multiplication assignment operator
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @param  scalar_ The scalar.
         *
         * @returns The result of the operation.
         */
        type operator*=(const value_type& scalar_) {
            return scale(scalar_);
        }

        /**
         * Division assignment operator
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @param  scalar_ The scalar.
         *
         * @returns The result of the operation.
         */
        type operator/=(const value_type& scalar_) {
            return scale(1 / scalar_);
        }

        /**
         * Addition operator
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @param  other_ The other.
         *
         * @returns The result of the operation.
         */
        type operator+(const_reference_type other_) const {
            return type(x + other_.x, y + other_.y, z + other_.z);
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
        type operator+=(const_reference_type other_) {
            x += other_.x;
            y += other_.y;
            z += other_.z;
            return *this;
        }

        /**
         * Subtraction operator
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @param  other_ The other.
         *
         * @returns The result of the operation.
         */
        type operator-(const_reference_type other_) const {
            return vec3_t<T>(x - other_.x, y - other_.y, z - other_.z);
        }

        /**
         * Negation operator
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @returns The result of the operation.
         */
        type operator-() const {
            return type(-x, -y, -z);
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
        type operator-=(const_reference_type other_) {
            x -= other_.x;
            y -= other_.y;
            z -= other_.z;
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
        type operator*(const_reference_type other_) const {
            return vec3_t<T>(x * other_.x, y * other_.y, z * other_.z);
        }

        /**
         * Multiplication operator
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @param  scalar_ The scalar.
         *
         * @returns The result of the operation.
         */
        type operator*(const value_type& scalar_) const {
            return type(x * scalar_, y * scalar_, z * scalar_);
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
        type operator/(const_reference_type other_) const {
            return type(x / other_.x, y / other_.y, z / other_.z);
        }

        /**
         * Division operator
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @param  scalar_ The scalar.
         *
         * @returns The result of the operation.
         */
        type operator/(const value_type& scalar_) const {
            return type(x / scalar_, y / scalar_, z / scalar_);
        }

        /**
         * Equality operator
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @param  other_ The other.
         *
         * @returns True if the parameters are considered equivalent.
         */
        bool operator==(const_reference_type other_) const {
            return (x == other_.x && y == other_.y && z == other_.z);
        }

        /**
         * Inequality operator
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @param  other_ The other.
         *
         * @returns True if the parameters are not considered equivalent.
         */
        bool operator!=(const_reference_type other_) const {
            return !(x == other_.x && y == other_.y && z == other_.z);
        }

        /**
         * Array indexer operator
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @param  index_ Zero-based index of the.
         *
         * @returns The indexed value.
         */
        value_type operator[](const size_t index_) const {
            return (&x)[index_];
        }

        /**
         * Array indexer operator
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @param  index_ Zero-based index of the.
         *
         * @returns The indexed value.
         */
        value_type& operator[](const size_t index_) {
            return (&x)[index_];
        }

        /**
         * An unnamed.
         *
         * @author Julius
         * @date 16.01.2020
         */
        union {

            /**
             * An unnamed.
             *
             * @author Julius
             * @date 16.01.2020
             */
            struct {
                T x, y, z;
            };

            /**
             * An unnamed.
             *
             * @author Julius
             * @date 16.01.2020
             */
            struct {
                T r, g, b;
            };

            /**
             * An unnamed.
             *
             * @author Julius
             * @date 16.01.2020
             */
            struct {
                T u, v, w;
            };
        };
    };

    /**
     * A vector 4 t.
     *
     * @author Julius
     * @date 16.01.2020
     *
     * @tparam T Generic type parameter.
     */
    template <typename T>
    class vec4_t {
    public:
        /** Defines an alias representing type of the value */
        typedef T value_type;
        /** Defines an alias representing the type */
        typedef vec4_t<value_type> type;
        /** Defines an alias representing type of the reference */
        typedef type& reference_type;
        /** Defines an alias representing type of the constant reference */
        typedef const type& const_reference_type;

        /**
         * Default constructor
         *
         * @author Julius
         * @date 16.01.2020
         */
        constexpr vec4_t() :
            x(0),
            y(0),
            z(0),
            w(0) { }

        /**
         * Constructor
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @param  x_ The x coordinate.
         * @param  y_ The y coordinate.
         * @param  z_ The z coordinate.
         * @param  w_ The w.
         */
        vec4_t(const value_type& x_, const value_type& y_, const value_type& z_, const value_type& w_) :
            x(x_),
            y(y_),
            z(z_),
            w(w_) { }

        /**
         * Constructor
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @param  scalar_ The scalar.
         */
        vec4_t(const value_type& scalar_) :
            x(scalar_),
            y(scalar_),
            z(scalar_),
            w(scalar_) { }

        /**
         * Constructor
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @param  xy_ The xy.
         * @param  zw_ The zw.
         */
        vec4_t(typename vec2_t<value_type>::const_reference_type xy_,
            typename vec2_t<value_type>::const_reference_type zw_) :
            x(xy_.x),
            y(xy_.y),
            z(zw_.x),
            w(zw_.x) { }

        /**
         * Constructor
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @param  xyz_ The xy.
         * @param  w_   The z coordinate.
         */
        vec4_t(typename vec3_t<value_type>::const_reference_type& xyz_, const value_type& w_) :
            x(xyz_.x),
            y(xyz_.y),
            z(xyz_.z),
            w(w_) { }

        /**
         * Sets an x coordinate
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @param  x_ The x coordinate.
         *
         * @returns A vec4&lt;T&gt;
         */
        reference_type setX(const value_type& x_) {
            x = x_;
            return *this;
        }

        /**
         * Sets a y coordinate
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @param  y_ The y coordinate.
         *
         * @returns A reference to a vec4&lt;T&gt;
         */
        reference_type setY(const value_type& y_) {
            y = y_;
            return *this;
        }

        /**
         * Sets a z coordinate
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @param  z_ The z coordinate.
         *
         * @returns A reference to a vec4&lt;T&gt;
         */
        reference_type setZ(const value_type& z_) {
            z = z_;
            return *this;
        }

        /**
         * Sets a w
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @param  w_ The w.
         *
         * @returns A reference to a vec4&lt;T&gt;
         */
        reference_type setW(const value_type& w_) {
            w = w_;
            return *this;
        }

        /**
         * Sets xyz to given vec3 values
         *
         * @author Julius
         * @date 23.05.2020
         *
         * @param  xyz_ The vec3 with xyz.
         *
         * @returns A reference to a vec4<T>;
         */
        reference_type set(typename vec3_t<value_type>::const_reference_type xyz_) {
            x = xyz_.x;
            y = xyz_.y;
            z = xyz_.z;
            return *this;
        }

        /**
         * Gets the length
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @returns A T.
         */
        value_type length() const {
            return static_cast<value_type>(std::sqrt(x * x + y * y + z * z + w * w));
        }

        /**
         * Dots the given other
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @param  other_ The other.
         *
         * @returns A T.
         */
        value_type dot(const_reference_type other_) const {
            const type tmp(*this * other_);
            return (tmp.x + tmp.y) + (tmp.z + tmp.w);
        }

        /**
         * Distances the given other
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @param  other_ The other.
         *
         * @returns A mathGenericType.
         */
        [[nodiscard]] mathGenericType distance(const_reference_type other_) const {
            return static_cast<mathGenericType>(std::sqrt(
                (x - other_.x) * (x - other_.x) + (y - other_.y) * (y - other_.y) + (z - other_.z) * (z - other_.z) + (w
                    - other_.w) * (w - other_.w)));
        }

        /**
         * Scales the given scale
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @param  scale_ The scale.
         *
         * @returns A vec4&lt;T&gt;
         */
        type scale(const value_type& scale_) {
            x *= scale_;
            y *= scale_;
            z *= scale_;
            w *= scale_;
            return *this;
        }

        /**
         * Gets the normalize
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @returns A vec4&lt;T&gt;
         */
        type normalize() {
            const value_type l = length();
            scale(1 / l);
            return *this;
        }

        /**
         * Multiplication assignment operator
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @param  scalar_ The scalar.
         *
         * @returns The result of the operation.
         */
        type operator*=(const value_type& scalar_) {
            return scale(scalar_);
        }

        /**
         * Division assignment operator
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @param  scalar_ The scalar.
         *
         * @returns The result of the operation.
         */
        type operator/=(const value_type& scalar_) {
            return scale(1 / scalar_);
        }

        /**
         * Addition operator
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @param  other_ The other.
         *
         * @returns The result of the operation.
         */
        type operator+(const_reference_type other_) const {
            return type(x + other_.x, y + other_.y, z + other_.z, w + other_.w);
        }

        /**
         * Subtraction operator
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @param  other_ The other.
         *
         * @returns The result of the operation.
         */
        type operator-(const_reference_type other_) const {
            return type(x - other_.x, y - other_.y, z - other_.z, w - other_.w);
        }

        /**
         * Negation operator
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @returns The result of the operation.
         */
        type operator-() const {
            return type(-x, -y, -z, -w);
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
        type operator*(const_reference_type other_) const {
            return type(x * other_.x, y * other_.y, z * other_.z, w * other_.w);
        }

        /**
         * Multiplication operator
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @param  scalar_ The scalar.
         *
         * @returns The result of the operation.
         */
        type operator*(const value_type& scalar_) const {
            return type(x * scalar_, y * scalar_, z * scalar_, w * scalar_);
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
        type operator/(const_reference_type other_) const {
            return type(x / other_.x, y / other_.y, z / other_.z, w / other_.w);
        }

        /**
         * Division operator
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @param  scalar_ The scalar.
         *
         * @returns The result of the operation.
         */
        type operator/(const value_type& scalar_) const {
            return type(x / scalar_, y / scalar_, z / scalar_, w / scalar_);
        }

        /**
         * Equality operator
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @param  other_ The other.
         *
         * @returns True if the parameters are considered equivalent.
         */
        bool operator==(const_reference_type other_) const {
            return (x == other_.x && y == other_.y && z == other_.z && w == other_.w);
        }

        /**
         * Inequality operator
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @param  other_ The other.
         *
         * @returns True if the parameters are not considered equivalent.
         */
        bool operator!=(const_reference_type other_) const {
            return !(x == other_.x && y == other_.y && z == other_.z && w == other_.w);
        }

        /**
         * Array indexer operator
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @param  index_ Zero-based index of the.
         *
         * @returns The indexed value.
         */
        value_type operator[](const size_t index_) const {
            return (&x)[index_];
        }

        /**
         * Array indexer operator
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @param  index_ Zero-based index of the.
         *
         * @returns The indexed value.
         */
        value_type& operator[](const size_t index_) {
            return (&x)[index_];
        }

        /**
         * Gets the xyz
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @returns A vec3_t&lt;T&gt;::type.
         */
        typename vec3_t<T>::type xyz() const {
            return vec3_t<T>(x, y, z);
        }

        /**
         * An unnamed.
         *
         * @author Julius
         * @date 16.01.2020
         */
        union {

            /**
             * An unnamed.
             *
             * @author Julius
             * @date 16.01.2020
             */
            struct {
                T x, y, z, w;
            };

            /**
             * An unnamed.
             *
             * @author Julius
             * @date 16.01.2020
             */
            struct {
                T r, g, b, a;
            };
        };
    };

    /** Defines an alias representing the second dvec */
    typedef vec2_t<double_t> dvec2;
    /** Defines an alias representing the second fvec */
    typedef vec2_t<float_t> fvec2;
    /** Defines an alias representing the second ivec */
    typedef vec2_t<s32> ivec2;
    /** Defines an alias representing the second uivec */
    typedef vec2_t<u32> uivec2;

    /** Defines an alias representing the third dvec */
    typedef vec3_t<double_t> dvec3;
    /** Defines an alias representing the third fvec */
    typedef vec3_t<float_t> fvec3;
    /** Defines an alias representing the third ivec */
    typedef vec3_t<s32> ivec3;
    /** Defines an alias representing the third uivec */
    typedef vec3_t<u32> uivec3;

    /** Defines an alias representing the fourth dvec */
    typedef vec4_t<double_t> dvec4;
    /** Defines an alias representing the fourth fvec */
    typedef vec4_t<float_t> fvec4;
    /** Defines an alias representing the fourth ivec */
    typedef vec4_t<s32> ivec4;
    /** Defines an alias representing the fourth uivec */
    typedef vec4_t<u32> uivec4;

    #ifdef MATH_DEFAULT_SINGLE_PRECISION
    /** The second vector */
    using vec2 = fvec2;
    /** The third vector */
    using vec3 = fvec3;
    /** The fourth vector */
    using vec4 = fvec4;
    #endif
    #ifdef MATH_DEFAULT_DOUBLE_PRECISION
	/** The second vector */
	using vec2 = dvec2;
	/** The third vector */
	using vec3 = dvec3;
	/** The fourth vector */
	using vec4 = dvec4;
    #endif

    /**
     * Subtraction operator
     *
     * @author Julius
     * @date 16.01.2020
     *
     * @param  l_ The first value.
     * @param  r_ A value to subtract from it.
     *
     * @returns The result of the operation.
     */
    template <typename T>
    vec2_t<T> operator-(typename vec2_t<T>::const_reference_type l_, typename vec2_t<T>::const_reference_type r_) {
        return vec2_t<T>(l_.x - r_.x, l_.y - r_.y);
    }

    /**
     * Addition operator
     *
     * @author Julius
     * @date 16.01.2020
     *
     * @param  l_ The first value.
     * @param  r_ A value to add to it.
     *
     * @returns The result of the operation.
     */
    template <typename T>
    vec4_t<T> operator+(typename vec4_t<T>::const_reference_type l_, typename vec4_t<T>::const_reference_type r_) {
        return vec4_t<T>(l_.x + r_.x, l_.y + r_.y, l_.z + r_.z, l_.w + r_.w);
    }

    /**
     * Multiplication operator
     *
     * @author Julius
     * @date 16.01.2020
     *
     * @param  l_ The first value to multiply.
     * @param  r_ The second value to multiply.
     *
     * @returns The result of the operation.
     */
    template <typename T>
    vec4_t<T> operator*(typename vec4_t<T>::const_reference_type l_, typename vec4_t<T>::const_reference_type r_) {
        return vec4_t<T>(l_.x * r_.x, l_.y * r_.y, l_.z * r_.z, l_.w * r_.w);
    }

    /**
     * Multiplication operator
     *
     * @author Julius
     * @date 16.01.2020
     *
     * @param  vec_    The first value to multiply.
     * @param  scalar_ The second value to multiply.
     *
     * @returns The result of the operation.
     */
    template <typename T>
    vec4_t<T> operator*(typename vec4_t<T>::const_reference_type vec_, const T& scalar_) {
        return (vec4_t<T>(vec_) *= scalar_);
    }

    /**
     * Multiplication operator
     *
     * @author Julius
     * @date 16.01.2020
     *
     * @param  scalar_ The first value to multiply.
     * @param  vec_    The second value to multiply.
     *
     * @returns The result of the operation.
     */
    template <typename T>
    vec3_t<T> operator*(const T& scalar_, typename vec3_t<T>::const_reference_type vec_) {
        return (vec3_t<T>(vec_) *= scalar_);
    }
}

/** . */
namespace std {

    /**
     * A vector 2 t.
     *
     * @author Julius
     * @date 16.01.2020
     *
     * @tparam T Generic type parameter.
     */
    template <typename T>
    struct hash<ember::math::vec2_t<T>> {

        /**
         * Function call operator
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @param  vec_ The vector.
         *
         * @returns The result of the operation.
         */
        size_t operator()(typename ember::math::vec2_t<T>::const_reference_type vec_) const {
            size_t seed = 0;
            hash<T> fnc;

            ember::hash::hash_combine(seed, fnc(vec_.x));
            ember::hash::hash_combine(seed, fnc(vec_.y));

            return seed;
        }
    };

    /**
     * A vector 3 t.
     *
     * @author Julius
     * @date 16.01.2020
     *
     * @tparam T Generic type parameter.
     */
    template <typename T>
    struct hash<ember::math::vec3_t<T>> {

        /**
         * Function call operator
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @param  vec_ The vector.
         *
         * @returns The result of the operation.
         */
        size_t operator()(typename ember::math::vec3_t<T>::const_reference_type vec_) const {
            size_t seed = 0;
            hash<T> fnc;

            ember::hash::hash_combine(seed, fnc(vec_.x));
            ember::hash::hash_combine(seed, fnc(vec_.y));
            ember::hash::hash_combine(seed, fnc(vec_.z));

            return seed;
        }
    };

    /**
     * A vector 4 t.
     *
     * @author Julius
     * @date 16.01.2020
     *
     * @tparam T Generic type parameter.
     */
    template <typename T>
    struct hash<ember::math::vec4_t<T>> {

        /**
         * Function call operator
         *
         * @author Julius
         * @date 16.01.2020
         *
         * @param  vec_ The vector.
         *
         * @returns The result of the operation.
         */
        size_t operator()(typename ember::math::vec4_t<T>::const_reference_type vec_) const {
            size_t seed = 0;
            hash<T> fnc;

            ember::hash::hash_combine(seed, fnc(vec_.x));
            ember::hash::hash_combine(seed, fnc(vec_.y));
            ember::hash::hash_combine(seed, fnc(vec_.z));
            ember::hash::hash_combine(seed, fnc(vec_.w));

            return seed;
        }
    };
}
