#pragma once
#include <type_traits>

#include "../Wrapper.hpp"

namespace hg {
    template <typename Ty_, typename StorageType_>
    struct NonOwningMemoryPointer;

    template <typename Type_>
    class SharedMemoryReference;
}

namespace hg {
    template <typename PointerType_>
    struct PointerKey {
        using this_type = PointerKey<PointerType_>;

        using raw_type = _STD remove_all_extents_t<PointerType_>;
        using pointer_type = ptr<raw_type>;

    public:
        constexpr PointerKey() noexcept :
            value(nullptr) {}

        constexpr PointerKey(const pointer_type value_) noexcept :
            value(value_) {}

        template <typename Type_> requires
            _STD is_same_v<Type_, PointerType_> &&
            _STD is_nothrow_convertible_v<Type_, pointer_type>
        constexpr PointerKey(Type_&& value_) noexcept :
            value(static_cast<pointer_type>(value_)) {}

        template <typename Type_, typename StorageType_>
        constexpr PointerKey(const NonOwningMemoryPointer<Type_, StorageType_>& value_) noexcept :
            value(static_cast<pointer_type>(value_.get())) {}

        template <typename Type_>
        constexpr PointerKey(const SharedMemoryReference<Type_>& value_) noexcept :
            value(static_cast<pointer_type>(value_.get())) {}

        constexpr PointerKey(cref<this_type> other_) noexcept :
            value(other_.value) {}

        constexpr PointerKey(mref<this_type> other_) noexcept :
            value(other_.value) {}

        constexpr ~PointerKey() noexcept = default;

        ref<this_type> operator=(cref<this_type> other_) noexcept {
            value = other_.value;
            return *this;
        }

        ref<this_type> operator=(mref<this_type> other_) noexcept {
            value = other_.value;
            return *this;
        }

        [[nodiscard]] constexpr bool operator==(cref<this_type> other_) const noexcept {
            return value == other_.value;
        }

        [[nodiscard]] constexpr bool operator!=(cref<this_type> other_) const noexcept {
            return value == other_.value;
        }

        [[nodiscard]] constexpr _STD strong_ordering operator<=>(cref<this_type> other_) const noexcept {
            return value <=> other_.value;
        }

        ptr<const void> value;
    };
}

namespace std {
    template <typename Type_>
    struct hash<::hg::PointerKey<Type_>> {
        [[nodiscard]] constexpr size_t operator()(auto&& value_) const noexcept {
            return reinterpret_cast<ptrdiff_t>(value_.value);
        }
    };
}
