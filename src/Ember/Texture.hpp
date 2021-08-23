#pragma once
#include <Engine.Common/Wrapper.hpp>

namespace ember {
    class Texture {
    private:
        /**
         * Internal
         */
        managed<void> _internal;

    public:
        /**
         * Gets the internal state
         *
         * @author Julius
         * @date 07.01.2021
         *
         * @returns A cref&lt;managed&lt;void&gt;&gt;
         */
        [[nodiscard]] cref<managed<void>> unwrap() const noexcept;
    };
}
