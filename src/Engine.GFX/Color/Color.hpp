#pragma once
#include <Engine.Common/Math/Vector.hpp>

namespace ember::engine {
    class color :
        public math::fvec4 {
    public:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 31.08.2020
         */
        color();

        /**
         * Constructor
         *
         * @author Julius
         * @date 31.08.2020
         *
         * @param  r_ The r.
         * @param  g_ The g.
         * @param  b_ The b.
         * @param  a_ (Optional) A.
         */
        color(const float& r_, const float& g_, const float& b_, const float& a_ = 1.F);

        /**
         * Constructor
         *
         * @author Julius
         * @date 31.08.2020
         *
         * @param  rgb_ The RGB.
         * @param  a_ (Optional) A.
         */
        color(const math::fvec3& rgb_, const float a_ = 1.F);

        /**
         * Constructor
         *
         * @author Julius
         * @date 31.08.2020
         *
         * @param  rgba_ The RGBA.
         */
        color(const math::fvec4& rgba_);

        /**
         * Gets the RGB space values
         *
         * @author Julius
         * @date 31.08.2020
         *
         * @returns A types::fvec3.
         */
        [[nodiscard]] math::fvec3 rgb() const;

        /**
         * Gets the RGBA space values
         *
         * @author Julius
         * @date 31.08.2020
         *
         * @returns A types::fvec4.
         */
        [[nodiscard]] math::fvec4 rgba() const;

        /**
         * Gets the HSL space values
         *
         * @author Julius
         * @date 31.08.2020
         *
         * @returns A types::fvec3.
         */
        [[nodiscard]] math::fvec3 hsl() const;

        /**
         * Gets the HSLA space values
         *
         * @author Julius
         * @date 31.08.2020
         *
         * @returns A types::fvec4.
         */
        [[nodiscard]] math::fvec4 hsla() const;

        /**
         * Converts this to a vector 3
         *
         * @author Julius
         * @date 17.11.2020
         *
         * @returns The result of the operation.
         */
        operator math::fvec3() const;

        /**
         * Converts this to a vector 4
         *
         * @author Julius
         * @date 17.11.2020
         *
         * @returns The result of the operation.
         */
        operator math::fvec4() const;
    };
}
