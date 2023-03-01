#pragma once

#include <Engine.Common/Wrapper.hpp>

#include "AssetInputStream.hpp"
#include "AssetOutputStream.hpp"

namespace hg::engine::assets {
    class AssetIOStream {
    public:
        using value_type = AssetIOStream;
        using reference_type = ref<value_type>;
        using const_reference_type = cref<value_type>;

    public:
        /**
         * Destructor
         *
         * @author Julius
         * @date 27.09.2021
         */
        ~AssetIOStream() noexcept;

    public:
        [[nodiscard]] operator ref<AssetInputStream>() noexcept;

        [[nodiscard]] operator ref<AssetOutputStream>() noexcept;
    };
}
