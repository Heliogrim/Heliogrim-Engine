#pragma once

/** . */
namespace ember::math {
    /**
     * A bezier 3 degree
     *
     * @author Julius
     * @date 30.01.2020
     *
     * @tparam Ty Type of the ty.
     */
    template <typename Ty>
    struct bezier3g {
        Ty p0, p1, p2, p3;

        /**
         * Calculate the bezier value for the given t_
         *
         * @author Julius
         * @date 30.01.2020
         *
         * @param t_ Clamped [0, 1] parameter for bezier function.
         *
         * @returns The result of the operation.
         */
        template <typename Py>
        Ty operator()(const Py& t_) const {
            const Py dt = static_cast<Py>(1) - t_;
            return (dt * dt * dt * p0) +
                (static_cast<Py>(3) * t_ * dt * dt * p1) +
                (static_cast<Py>(3) * t_ * t_ * dt * p2) +
                (t_ * t_ * t_ * p3);
        }
    };

    template <typename Ty>
    using bezier = bezier3g<Ty>;
}
