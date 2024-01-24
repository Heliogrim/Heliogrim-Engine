#pragma once

#include "../__macro.hpp"
#include <complex>

namespace hg::math {
    typedef std::complex<double> complex_t;
    typedef std::pair<complex_t, complex_t> complex_pair_t;

    template <typename Real>
    inline std::complex<Real> solve_quadratic_1(Real a_, Real b_, Real c_) {
        return (-b_ + sqrt(std::complex<Real>(b_ * b_ - 4 * a_ * c_, 0))) / (2.0 * a_);
    }

    template <typename Real>
    inline std::complex<Real> solve_quadratic_2(Real a_, Real b_, Real c_) {
        return (-b_ - sqrt(std::complex<Real>(b_ * b_ - 4 * a_ * c_, 0))) / (2.0 * a_);
    }

    inline const complex_t infinity() {
        return complex_t(std::numeric_limits<double>::infinity());
    }

    inline const complex_t adjust_imag(const complex_t& c_) {
        if (std::fabs(c_.imag()) < 1e-30) {
            return complex_t(c_.real(), 0);
        } else {
            return c_;
        }
    }

    template <typename Ty, typename To>
    inline std::complex<Ty> addmul(const std::complex<Ty>& c_, Ty v, const std::complex<To>& d_) {
        return std::complex<Ty>(c_.real() + v * d_.real(), c_.imag() + v * d_.imag());
    }

    template <typename Ty>
    inline bool is_nan(Ty v_) {
        return !(v_ == v_);
    }

    template <>
    inline bool is_nan<complex_t>(complex_t v_) {
        return is_nan(v_.real()) || is_nan(v_.imag());
    }

    constexpr double anti_denormal_vsa = 1e-8;

    class DenormalPrevention {
    public:
        DenormalPrevention() :
            _v(anti_denormal_vsa) { }

        inline double ac() {
            return _v = -_v;
        }

        static inline double dc() {
            return anti_denormal_vsa;
        }

    private:
        double _v;
    };
}
