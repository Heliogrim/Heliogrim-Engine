#pragma once

#include <corecrt_math.h>

#include "MathDefaultDefine.hpp"

#include "Vector.hpp"

namespace hg::math {
    template <typename T, std::size_t R, std::size_t C>
    class mat_t { };

    template <typename T, std::size_t R>
    class matq_t : public mat_t<T, R, R> { };

    template <typename T>
    class matq2_t :
        public matq_t<T, 2> {
    public:
        using value_type = T;
        typedef vec2_t<T> col_type;
        using row_type = col_type;

        /**
        * Constructor
        *
        * @param r0_ The 0.
        * @param r1_ The 1.
        */
        matq2_t(const row_type& r0_, const row_type& r1_) :
            value { r0_, r1_ } { }

        /**
        * Constructor
        *
        * @param scalar_ The scalar.
        */
        matq2_t(const T& scalar_) :
            matq2_t(row_type(scalar_), row_type(scalar_)) { }

        /** Default constructor */
        matq2_t() :
            matq2_t(static_cast<T>(0)) { }

        /**
        * Array indexer operator
        *
        * @param index_ Zero-based index of the.
        *
        * @return The indexed value.
        */
        row_type& operator[](const size_t index_) const {
            return value[index_];
        }

    private:
        row_type value[2];
    };

    template <typename T>
    class matq3_t :
        public matq_t<T, 3> {
    public:
        using value_type = T;
        typedef vec3_t<T> col_type;
        using row_type = col_type;

        /**
         * Constructor
         *
         * @param r0_ The 0.
         * @param r1_ The 1.
         * @param r2_ The 2.
         */
        matq3_t(const row_type& r0_, const row_type& r1_, const row_type& r2_) :
            value { r0_, r1_, r2_ } { }

        /**
         * Constructor
         *
         * @param scalar_ The scalar.
         */
        matq3_t(const T& scalar_) :
            matq3_t(row_type(scalar_), row_type(scalar_), row_type(scalar_)) { }

        /** Default constructor */
        matq3_t() :
            matq3_t(static_cast<T>(0)) { }

        /**
         * Array indexer operator
         *
         * @param index_ Zero-based index of the.
         *
         * @return The indexed value.
         */
        const row_type& operator[](const size_t index_) const {
            return value[index_];
        }

        /**
         * Array indexer operator
         *
         * @param index_ Zero-based index of the.
         *
         * @return The indexed value.
         */
        row_type& operator[](const size_t index_) {
            return value[index_];
        }

    private:
        row_type value[3];
    };

    template <typename T>
    class matq4_t :
        public matq_t<T, 4> {
    public:
        using value_type = T;
        typedef vec4_t<T> col_type;
        using row_type = col_type;

        /**
         * Constructor
         *
         * @param v00_ The 00.
         * @param v01_ The 01.
         * @param v02_ The 02.
         * @param v03_ The 03.
         * @param v10_ The 10.
         * @param v11_ The 11.
         * @param v12_ The 12.
         * @param v13_ The 13.
         * @param v20_ The 20.
         * @param v21_ The 21.
         * @param v22_ The 22.
         * @param v23_ The 23.
         * @param v30_ The 30.
         * @param v31_ The 31.
         * @param v32_ The 32.
         * @param v33_ The 33.
         */
        constexpr matq4_t(
            const T& v00_,
            const T& v01_,
            const T& v02_,
            const T& v03_,
            const T& v10_,
            const T& v11_,
            const T& v12_,
            const T& v13_,
            const T& v20_,
            const T& v21_,
            const T& v22_,
            const T& v23_,
            const T& v30_,
            const T& v31_,
            const T& v32_,
            const T& v33_
        ) :
            value {
                { v00_, v01_, v02_, v03_ },
                { v10_, v11_, v12_, v13_ },
                { v20_, v21_, v22_, v23_ },
                { v30_, v31_, v32_, v33_ }
            } { }

        /**
         * Constructor
         *
         * @param r0_ The 0.
         * @param r1_ The 1.
         * @param r2_ The 2.
         * @param r3_ The 3.
         */
        constexpr matq4_t(const row_type& r0_, const row_type& r1_, const row_type& r2_, const row_type& r3_) :
            value { r0_, r1_, r2_, r3_ } { }

        /**
         * Constructor
         *
         * @param scalar_ The scalar.
         */
        constexpr matq4_t(const T& scalar_) :
            value {
                row_type(scalar_, 0, 0, 0),
                row_type(0, scalar_, 0, 0),
                row_type(0, 0, scalar_, 0),
                row_type(0, 0, 0, scalar_)
            } { }

        /** Default constructor */
        constexpr matq4_t() :
            matq4_t(0) { }

        /**
         * Constructor
         *
         * @param m3_ The 3x3 matrix to convert
         */
        constexpr matq4_t(const matq3_t<T> m3_) :
            value {
                row_type { m3_[0], 0 },
                row_type { m3_[1], 0 },
                row_type { m3_[2], 0 },
                row_type { 0, 0, 0, 1 }
            } {}

        /**
         * Scales the given scalar
         *
         * @param scalar_ The scalar.
         *
         * @return A matq4_t&lt;T&gt;
         */
        matq4_t<T>& scale(const T& scalar_) {
            value[0] *= scalar_;
            value[1] *= scalar_;
            value[2] *= scalar_;
            value[3] *= scalar_;

            return *this;
        }

        /**
         * Scaled the given scalar
         *
         * @param scalar_ The scalar.
         *
         * @return A matq4_t&lt;T&gt;
         */
        matq4_t<T> scaled(const T& scalar_) const {
            matq4_t<T> res = matq4_t<T>(*this);
            return res.scale(scalar_);
        }

        matq4_t<T>& unchecked_scale(const vec3_t<T> scale_) {
            value[0] *= scale_[0];
            value[1] *= scale_[1];
            value[2] *= scale_[2];
            //value[3] *= scale_[3];

            return *this;
        }

        matq4_t<T> unchecked_scaled(const vec3_t<T> scale_) {
            matq4_t<T> res = matq4_t<T>(*this);
            return res.unchecked_scale(scale_);
        }

        /**
         * Rotates
         *
         * @param angle_ The angle.
         * @param vec_   The vector.
         *
         * @return A matq4_t&lt;T&gt;
         */
        matq4_t<T>& rotate(const T& angle_, const vec3_t<T> vec_) {
            auto rm = rotated(angle_, vec_);
            *this = _STD move(rm);
            return *this;
        }

        /**
         * Rotates
         *
         * @param angle_ The angle.
         * @param vec_   The vector.
         *
         * @return A matq4_t&lt;T&gt;
         */
        matq4_t<T> rotated(const T& angle_, const vec3_t<T> vec_) const {
            const T a = angle_;
            const T c = _STD cos(a);
            const T s = _STD sin(a);

            const vec3_t<T> axis = vec_.normalized();
            vec3_t<T> tmp = vec3_t<T>((T(1) - c) * axis);
            matq4_t<T> hm = matq4_t<T>();

            hm[0][0] = c + tmp[0] * axis[0];
            hm[0][1] = tmp[0] * axis[1] + s * axis[2];
            hm[0][2] = tmp[0] * axis[2] - s * axis[1];

            hm[1][0] = tmp[1] * axis[0] - s * axis[2];
            hm[1][1] = c + tmp[1] * axis[1];
            hm[1][2] = tmp[1] * axis[2] + s * axis[0];

            hm[2][0] = tmp[2] * axis[0] + s * axis[1];
            hm[2][1] = tmp[2] * axis[1] - s * axis[0];
            hm[2][2] = c + tmp[2] * axis[2];

            matq4_t<T> res = matq4_t<T>();
            res[0] = (value[0] * hm[0][0]) + (value[1] * hm[0][1]) + (value[2] * hm[0][2]);
            res[1] = (value[0] * hm[1][0]) + (value[1] * hm[1][1]) + (value[2] * hm[1][2]);
            res[2] = (value[0] * hm[2][0]) + (value[1] * hm[2][1]) + (value[2] * hm[2][2]);
            res[3] = value[3];

            return res;
        }

        /**
         * Translates the given vector
         *
         * @param vec_ The vector.
         *
         * @return A reference to a matq4_t&lt;T&gt;
         */
        matq4_t<T>& translate(const vec3_t<T>& vec_) {
            value[3] = value[0] * vec_[0] + value[1] * vec_[1] + value[2] * vec_[2] + value[3];
            return *this;
        }

        /**
         * Translated the given vector
         *
         * @param vec_ The vector.
         *
         * @return A matq4_t&lt;T&gt;
         */
        matq4_t<T> translated(const vec3_t<T>& vec_) const {
            matq4_t<T> res = matq4_t<T>(*this);
            return res.translate(vec_);
        }

        /**
         * Gets the inverse
         *
         * @return A matq4_t&lt;T&gt;
         */
        matq4_t<T> inverse() const {
            T c00 = value[2][2] * value[3][3] - value[3][2] * value[2][3];
            T c02 = value[1][2] * value[3][3] - value[3][2] * value[1][3];
            T c03 = value[1][2] * value[2][3] - value[2][2] * value[1][3];

            T c04 = value[2][1] * value[3][3] - value[3][1] * value[2][3];
            T c06 = value[1][1] * value[3][3] - value[3][1] * value[1][3];
            T c07 = value[1][1] * value[2][3] - value[2][1] * value[1][3];

            T c08 = value[2][1] * value[3][2] - value[3][1] * value[2][2];
            T c10 = value[1][1] * value[3][2] - value[3][1] * value[1][2];
            T c11 = value[1][1] * value[2][2] - value[2][1] * value[1][2];

            T c12 = value[2][0] * value[3][3] - value[3][0] * value[2][3];
            T c14 = value[1][0] * value[3][3] - value[3][0] * value[1][3];
            T c15 = value[1][0] * value[2][3] - value[2][0] * value[1][3];

            T c16 = value[2][0] * value[3][2] - value[3][0] * value[2][2];
            T c18 = value[1][0] * value[3][2] - value[3][0] * value[1][2];
            T c19 = value[1][0] * value[2][2] - value[2][0] * value[1][2];

            T c20 = value[2][0] * value[3][1] - value[3][0] * value[2][1];
            T c22 = value[1][0] * value[3][1] - value[3][0] * value[1][1];
            T c23 = value[1][0] * value[2][1] - value[2][0] * value[1][1];

            vec4_t<T> fac0(c00, c00, c02, c03);
            vec4_t<T> fac1(c04, c04, c06, c07);
            vec4_t<T> fac2(c08, c08, c10, c11);
            vec4_t<T> fac3(c12, c12, c14, c15);
            vec4_t<T> fac4(c16, c16, c18, c19);
            vec4_t<T> fac5(c20, c20, c22, c23);

            vec4_t<T> vec0(value[1][0], value[0][0], value[0][0], value[0][0]);
            vec4_t<T> vec1(value[1][1], value[0][1], value[0][1], value[0][1]);
            vec4_t<T> vec2(value[1][2], value[0][2], value[0][2], value[0][2]);
            vec4_t<T> vec3(value[1][3], value[0][3], value[0][3], value[0][3]);

            vec4_t<T> inv0(vec1 * fac0 - vec2 * fac1 + vec3 * fac2);
            vec4_t<T> inv1(vec0 * fac0 - vec2 * fac3 + vec3 * fac4);
            vec4_t<T> inv2(vec0 * fac1 - vec1 * fac3 + vec3 * fac5);
            vec4_t<T> inv3(vec0 * fac2 - vec1 * fac4 + vec2 * fac5);

            vec4_t<T> sA(+1, -1, +1, -1);
            vec4_t<T> sB(-1, +1, -1, +1);
            matq4_t<T> inv(inv0 * sA, inv1 * sB, inv2 * sA, inv3 * sB);

            vec4_t<T> r(inv[0][0], inv[1][0], inv[2][0], inv[3][0]);
            vec4_t<T> d(value[0] * r);
            T dv = (d.x + d.y) + (d.z + d.w);
            T ood = static_cast<T>(1) / dv;

            return (inv * ood);
        }

        /**
         * @return The result of the operation.
         */
        matq4_t<T>& identity() {
            value[0] = row_type(1, 0, 0, 0);
            value[1] = row_type(0, 1, 0, 0);
            value[2] = row_type(0, 0, 1, 0);
            value[3] = row_type(0, 0, 0, 1);
            return *this;
        }

        /**
         * \brief 
         * \return 
         */
        constexpr static matq4_t<T> make_identity() {
            return matq4_t(1);
        }

        /**
         * Array indexer operator
         *
         * @param index_ Zero-based index of the.
         *
         * @return The indexed value.
         */
        const row_type& operator[](const size_t index_) const {
            return value[index_];
        }

        /**
         * Array indexer operator
         *
         * @param index_ Zero-based index of the.
         *
         * @return The indexed value.
         */
        row_type& operator[](const size_t index_) {
            return value[index_];
        }

        /**
         * Multiplication operator
         *
         * @param other_ The other.
         *
         * @return The result of the operation.
         */
        matq4_t<T> operator*(const matq4_t<T>& other_) const {
            return matq4_t<T>(
                value[0] * other_[0][0] + value[1] * other_[0][1] + value[2] * other_[0][2] + value[3] * other_[0][3],
                value[0] * other_[1][0] + value[1] * other_[1][1] + value[2] * other_[1][2] + value[3] * other_[1][3],
                value[0] * other_[2][0] + value[1] * other_[2][1] + value[2] * other_[2][2] + value[3] * other_[2][3],
                value[0] * other_[3][0] + value[1] * other_[3][1] + value[2] * other_[3][2] + value[3] * other_[3][3]
            );
        }

        /**
         * Multiplication assignment operator
         *
         * @param scalar_ The scalar.
         *
         * @return The result of the operation.
         */
        matq4_t<T>& operator*=(const T& scalar_) {
            value[0] *= scalar_;
            value[1] *= scalar_;
            value[2] *= scalar_;
            value[3] *= scalar_;
            return *this;
        }

        /**
         * \brief 
         * \param right_ 
         * \return 
         */
        matq4_t<T>& operator*=(const matq4_t<T>& right_) { }

        /**
         * Division assignment operator
         *
         * @param scalar_ The scalar.
         *
         * @return The result of the operation.
         */
        matq4_t<T>& operator/=(const T& scalar_) {
            value[0] /= scalar_;
            value[1] /= scalar_;
            value[2] /= scalar_;
            value[3] /= scalar_;
            return *this;
        }

    private:
        row_type value[4];
    };

    typedef matq2_t<float_t> fmat2;
    typedef matq3_t<float_t> fmat3;
    typedef matq4_t<float_t> fmat4;

    typedef matq2_t<double_t> dmat2;
    typedef matq3_t<double_t> dmat3;
    typedef matq4_t<double_t> dmat4;

    #ifdef MATH_DEFAULT_SINGLE_PRECISION
    using mat2 = fmat2;
    using mat3 = fmat3;
    using mat4 = fmat4;
    #endif
    #ifdef MATH_DEFAULT_DOUBLE_PRECISION
	using mat2 = dmat2;
	using mat3 = dmat3;
	using mat4 = dmat4;
    #endif

    template <typename T>
    typename matq4_t<T>::col_type operator*(const matq4_t<T>& mat_, const typename matq4_t<T>::row_type& vec_) {
        const typename matq4_t<T>::col_type mo0(vec_[0]);
        const typename matq4_t<T>::col_type mo1(vec_[1]);
        const typename matq4_t<T>::col_type mu0 = mat_[0] * mo0;
        const typename matq4_t<T>::col_type mu1 = mat_[1] * mo1;
        const typename matq4_t<T>::col_type ad0 = mu0 + mu1;

        const typename matq4_t<T>::col_type mo2(vec_[2]);
        const typename matq4_t<T>::col_type mo3(vec_[3]);
        const typename matq4_t<T>::col_type mu2 = mat_[2] * mo2;
        const typename matq4_t<T>::col_type mu3 = mat_[3] * mo3;
        const typename matq4_t<T>::col_type ad1 = mu2 + mu3;

        const typename matq4_t<T>::col_type ad2 = ad0 + ad1;

        return ad2;
    }

    template <typename T>
    typename matq4_t<T>::row_type operator*(const typename matq4_t<T>::col_type& vec_, const matq4_t<T>& mat_) {
        return typename matq4_t<T>::row_type(
            mat_[0][0] * vec_[0] + mat_[0][1] * vec_[1] + mat_[0][2] * vec_[2] + mat_[0][3] * vec_[3],
            mat_[1][0] * vec_[0] + mat_[1][1] * vec_[1] + mat_[1][2] * vec_[2] + mat_[1][3] * vec_[3],
            mat_[2][0] * vec_[0] + mat_[2][1] * vec_[1] + mat_[2][2] * vec_[2] + mat_[2][3] * vec_[3],
            mat_[3][0] * vec_[0] + mat_[3][1] * vec_[1] + mat_[3][2] * vec_[2] + mat_[3][3] * vec_[3]
        );
    }

    template <typename T>
    typename matq3_t<T>::row_type operator*(const typename matq3_t<T>::col_type& vec_, const matq3_t<T>& mat_) {

        const typename matq3_t<T>::col_type res {
            mat_[0].dot(vec_),
            mat_[1].dot(vec_),
            mat_[2].dot(vec_)
        };

        return res;
    }

    template <typename T>
    [[maybe_unused]] vec3_t<T> operator*(const vec3_t<T>& vec_, const matq4_t<T>& mat_) {
        return vec3_t<T>(
            mat_[0][0] * vec_[0] + mat_[0][1] * vec_[1] + mat_[0][2] * vec_[2],
            mat_[1][0] * vec_[0] + mat_[1][1] * vec_[1] + mat_[1][2] * vec_[2],
            mat_[2][0] * vec_[0] + mat_[2][1] * vec_[1] + mat_[2][2] * vec_[2]
        );
    }

    template <typename T>
    [[maybe_unused]] static matq4_t<T> lookAt(const vec3_t<T> eye_, const vec3_t<T>& center_, const vec3_t<T>& up_) {
        const vec3_t<T> f { (center_ - eye_).normalize() };
        const vec3_t<T> s { (f.crossed(up_)).normalize() };
        const vec3_t<T> u { s.crossed(f) };

        matq4_t<T> res = matq4_t<T>(1);
        res[0][0] = s.x;
        res[1][0] = s.y;
        res[2][0] = s.z;

        res[0][1] = u.x;
        res[1][1] = u.y;
        res[2][1] = u.z;

        res[0][2] = -f.x;
        res[1][2] = -f.y;
        res[2][2] = -f.z;

        res[3][0] = -s.dot(eye_);
        res[3][1] = -u.dot(eye_);
        res[3][2] = f.dot(eye_);

        return res;
    }

    template <typename T>
    [[maybe_unused]] static matq4_t<T> ortho(
        const T& l_,
        const T& r_,
        const T& b_,
        const T& t_,
        const T& znear_,
        const T& zfar_
    ) {
        matq4_t<T> res = matq4_t<T>(1);
        res[0][0] = static_cast<T>(2) / (r_ - l_);
        res[1][1] = static_cast<T>(2) / (t_ - b_);
        res[2][2] = static_cast<T>(2) / (zfar_ - znear_);
        res[3][0] = -(r_ + l_) / (r_ - l_);
        res[3][1] = -(t_ + b_) / (t_ - b_);
        res[3][2] = -(zfar_ + znear_) / (zfar_ - znear_);

        return res;
    }

    template <typename T>
    [[maybe_unused]] static matq4_t<T> ortho(const T& l_, const T& r_, const T& b_, const T& t_) {
        matq4_t<T> res = matq4_t<T>(1);
        res[0][0] = static_cast<T>(2) / (r_ - l_);
        res[1][1] = static_cast<T>(2) / (t_ - b_);
        res[2][2] = static_cast<T>(1);
        res[3][0] = -(r_ + l_) / (r_ - l_);
        res[3][1] = -(t_ + b_) / (t_ - b_);

        return res;
    }

    [[maybe_unused]] static mat4 perspective(
        const mat4::value_type& fov_,
        const mat4::value_type& aspect_,
        const mat4::value_type& znear_,
        const mat4::value_type& zfar_
    ) {
        const mat4::value_type ht = tan(fov_ / static_cast<mat4::value_type>(2));

        mat4 res = mat4(0);
        res[0][0] = static_cast<mat4::value_type>(1) / (aspect_ * ht);
        res[1][1] = static_cast<mat4::value_type>(1) / (ht);
        res[2][2] = zfar_ / (znear_ - zfar_);
        res[2][3] = - static_cast<mat4::value_type>(1);
        res[3][2] = -(zfar_ * znear_) / (zfar_ - znear_);

        return res;
    }
}
