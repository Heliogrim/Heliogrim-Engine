#pragma once

#include <combaseapi.h>

#include "Types.hpp"
#include "Wrapper.hpp"
#include "__macro.hpp"

namespace ember {

    struct Uuid {

        constexpr Uuid() noexcept :
            data(0) {}

        explicit constexpr Uuid(const u64& value_) :
            data(value_) {}

        explicit Uuid(const GUID& value_) :
            data(reinterpret_cast<const u64&>(value_)) {}

        [[nodiscard]] FORCE_INLINE bool operator>(const Uuid& other_) const noexcept {
            return data > other_.data;
        }

        [[nodiscard]] FORCE_INLINE bool operator<(const Uuid& other_) const noexcept {
            return data < other_.data;
        }

        [[nodiscard]] FORCE_INLINE bool operator>=(const Uuid& other_) const noexcept {
            return data >= other_.data;
        }

        [[nodiscard]] FORCE_INLINE bool operator<=(const Uuid& other_) const noexcept {
            return data <= other_.data;
        }

        [[nodiscard]] FORCE_INLINE bool operator==(const Uuid& other_) const noexcept {
            return data == other_.data;
        }

        [[nodiscard]] FORCE_INLINE bool operator!=(const Uuid& other_) const noexcept {
            return data != other_.data;
        }

        u64 data;
    };

    inline void UuidGenerate(ref<Uuid> uuid_) {
        #ifdef _WIN32
        CoCreateGuid(reinterpret_cast<GUID*>(&uuid_.data));
        #else
        #pragma error("Not implemented uuid generator.")
        #endif
    }
}
