#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Flag.hpp>

namespace hg {
    enum class AssetDatabaseResultType : u8 {
        eFailed = 0b0000,
        eSuccess = 0b0001,
        //
        eLoading = 0b0011
    };

    using AssetDatabaseResultFlags = Flag<AssetDatabaseResultType>;

    template <typename ValueType_ = ptr<void>>
    struct AssetDatabaseResult {
        using value_type = std::remove_cvref_t<ValueType_>;

        constexpr explicit AssetDatabaseResult(
            cref<AssetDatabaseResultFlags> flags_,
            cref<value_type> value_
        ) :
            flags(flags_),
            value(value_) {}

        constexpr explicit AssetDatabaseResult(
            mref<AssetDatabaseResultFlags> flags_,
            mref<value_type> value_
        ) :
            flags(std::move(flags_)),
            value(std::move(value_)) {}

        AssetDatabaseResultFlags flags;
        value_type value;

        /**
         * Checks whether the result type is successful
         *
         * @author Julius
         * @date 03.10.2021
         *
         * @returns True if the result type is successful, otherwise false.
         */
        [[nodiscard]] operator bool() const noexcept {
            return flags & AssetDatabaseResultType::eSuccess;
        }

        template <typename DstValueType_ = ptr<void>> requires std::is_convertible_v<
            value_type, typename AssetDatabaseResult<DstValueType_>::value_type>
        [[nodiscard]] operator AssetDatabaseResult<DstValueType_>() const noexcept(
            std::is_nothrow_convertible_v<value_type, typename AssetDatabaseResult<DstValueType_>::value_type>
        ) {
            return AssetDatabaseResult<DstValueType_> {
                flags,
                static_cast<typename AssetDatabaseResult<DstValueType_>::value_type>(value)
            };
        }
    };
}
