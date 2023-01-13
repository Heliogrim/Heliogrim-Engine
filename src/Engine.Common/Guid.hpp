#pragma once

#include <combaseapi.h>

#include "Types.hpp"
#include "Wrapper.hpp"
#include "__macro.hpp"
#include "Integral128.hpp"

namespace ember {
    struct Guid {
        constexpr Guid() noexcept :
            data() {}

        constexpr Guid(const Guid& other_) noexcept :
            data(other_.data) {}

        constexpr Guid(Guid&& other_) noexcept :
            data(_STD move(other_.data)) {}

        constexpr Guid(const uint128_t& data_) noexcept:
            data(data_) {}

        template <
            _STD integral PreType_,
            _STD integral C0Type_,
            _STD integral C1Type_,
            _STD integral PostType_
        >
        constexpr Guid(const PreType_& pre_, const C0Type_& c0_, const C1Type_& c1_, const PostType_& post_) :
            pre(pre_),
            c0(c0_),
            c1(c1_),
            post(post_) {}

        Guid(const byte (&bytes_)[16]) :
            Guid() {
            _STD memcpy(bytes, bytes_, 16);
        }

        explicit Guid(const GUID& value_) :
            pre(value_.Data1),
            c0(value_.Data2),
            c1(value_.Data3),
            post(*reinterpret_cast<const u64* const>(value_.Data4)) {}

        constexpr ref<Guid> operator=(cref<Guid> other_) noexcept {
            data = other_.data;
            return *this;
        }

        constexpr ref<Guid> operator=(mref<Guid> other_) noexcept {
            data = _STD move(other_.data);
            return *this;
        }

        [[nodiscard]] operator bool() const noexcept {
            return data.operator bool();
        }

        [[nodiscard]] bool operator!() const noexcept {
            return !data;
        }

        [[nodiscard]] FORCE_INLINE bool operator>(const Guid& other_) const noexcept {
            return data.low > other_.data.low && data.high > other_.data.high;
        }

        [[nodiscard]] FORCE_INLINE bool operator<(const Guid& other_) const noexcept {
            return data.low < other_.data.low && data.high < other_.data.high;
        }

        [[nodiscard]] FORCE_INLINE bool operator>=(const Guid& other_) const noexcept {
            return data.low >= other_.data.low && data.high >= other_.data.high;
        }

        [[nodiscard]] FORCE_INLINE bool operator<=(const Guid& other_) const noexcept {
            return data.low <= other_.data.low && data.high <= other_.data.high;
        }

        [[nodiscard]] FORCE_INLINE bool operator==(const Guid& other_) const noexcept {
            return data == other_.data;
        }

        [[nodiscard]] FORCE_INLINE bool operator!=(const Guid& other_) const noexcept {
            return data != other_.data;
        }

        union {
            struct {
                uint128_t data;
            };

            struct {
                u32 pre;
                u16 c0;
                u16 c1;
                u64 post;
            };

            struct {
                u8 bytes[16];
            };
        };
    };

    inline void GuidGenerate(ref<Guid> guid_) {
        #ifdef _WIN32
        static_assert(sizeof(Guid) == sizeof(GUID));
        CoCreateGuid(reinterpret_cast<GUID*>(&guid_.bytes));
        #else
        #pragma error("Not implemented uuid generator.")
        #endif
    }
}
