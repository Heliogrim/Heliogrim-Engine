#pragma once
#include <cassert>
#include <type_traits>
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.GFX.Render.Command/NativeQueue.hpp>

namespace hg::driver::vk {
    enum class VkQueueTypeBits {
        eGraphics = 0x1 << 28,
        eCompute = 0x1 << 29,
        eTransfer = 0x1 << 30
    };

    struct VkQueueMask {
        using type_bit_type = VkQueueTypeBits;
        using index_type = u32;
        using value_type = u32;

        value_type mask;

        constexpr void __range_check(auto&& val_) const noexcept {
            assert(val_ < 28uL);
        }

        constexpr auto __type_value(auto&& val_) const noexcept {
            return static_cast<value_type>(val_);
        }

        [[nodiscard]] constexpr bool has(const type_bit_type typeBit_) const noexcept {
            return mask & __type_value(typeBit_);
        }

        [[nodiscard]] constexpr bool has(const index_type index_) const noexcept {
            __range_check(index_);
            return mask & (0x1 << index_);
        }

        ref<VkQueueMask> store(const type_bit_type typeBit_) noexcept {
            mask |= __type_value(typeBit_);
            return *this;
        }

        ref<VkQueueMask> store(const index_type index_) noexcept {
            __range_check(index_);
            mask |= (0x1 << index_);
            return *this;
        }

        ref<VkQueueMask> drop(const type_bit_type typeBit_) noexcept {
            mask &= ~(__type_value(typeBit_));
            return *this;
        }

        ref<VkQueueMask> drop(const index_type index_) noexcept {
            __range_check(index_);
            mask &= ~(0x1 << index_);
            return *this;
        }
    };

    static_assert(
        _STD is_same_v<
            VkQueueMask::value_type,
            _STD make_unsigned_t<_STD underlying_type_t<VkQueueTypeBits>>
        >
    );

    /**/

    class VkNativeQueue :
        public engine::render::cmd::NativeQueue {
    public:
        using this_type = VkNativeQueue;

    public:
        constexpr VkNativeQueue() noexcept = default;

        constexpr ~VkNativeQueue() noexcept = default;

    public:
        [[nodiscard]] VkQueueMask::index_type getQueueIndex() const noexcept;

        [[nodiscard]] VkQueueTypeBits getQueueType() const noexcept;
    };
}
