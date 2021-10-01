#pragma once

#include <Engine.Common/Wrapper.hpp>

namespace ember::engine::assets {

    class AssetOutputStream {
    public:
        using value_type = AssetOutputStream;
        using reference_type = ref<value_type>;
        using const_reference_type = cref<value_type>;

    public:
        /**
         * Destructor
         *
         * @author Julius
         * @date 27.09.2021
         */
        ~AssetOutputStream() noexcept;
    };
}
