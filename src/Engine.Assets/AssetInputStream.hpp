#pragma once

#include <Engine.Common/Wrapper.hpp>

namespace ember::engine::assets {
    class AssetInputStream {
    public:
        using value_type = AssetInputStream;
        using reference_type = ref<value_type>;
        using const_reference_type = cref<value_type>;

    public:
        /**
         * Destructor
         *
         * @author Julius
         * @date 27.09.2021
         */
        ~AssetInputStream() noexcept;
    };
}
