#pragma once

#include <atomic>
#include <concepts>
#include <functional>
#include <type_traits>

#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>

namespace hg::engine::scheduler::fiber {
    /**
     * await_signal_type :: Defines type to use as awaitable signal by source
     * await_signal_sub_type :: Defines the subscription type the FiberAwaitable will use
     *
     *  Be aware that `std::atomic_flag` had no `const std::atomic_flag::test()` before CXX20
     *      which required implicitly a mutable object due to `std::atomic_flag::test_and_set()`
     */
    using await_signal_type = std::atomic_flag;
    using await_signal_sub_type = const await_signal_type;

    template <typename AwaitableType_>
    concept IsAwaitableSignal = std::is_same_v<AwaitableType_, await_signal_type> || std::is_same_v<AwaitableType_,
        await_signal_sub_type>;

    template <typename AwaitableType_>
    concept IsAwaitableSignalRet = requires(const AwaitableType_ obj) {
            { obj.await() } -> std::same_as<const ptr<await_signal_type>>;
        } || requires(const AwaitableType_ obj) {
            { obj.await() } -> std::same_as<const ptr<await_signal_sub_type>>;
        };

    template <typename AwaitableType_>
    concept IsAwaitableSignalCall = requires(const AwaitableType_ obj) {
        { obj.await() } -> std::same_as<bool>;
    };

    template <typename AwaitableType_>
    concept IsAwaitable = IsAwaitableSignal<AwaitableType_> || IsAwaitableSignalRet<AwaitableType_> ||
        IsAwaitableSignalCall<AwaitableType_>;

    enum class FiberAwaitableBits : u8 {
        eUndefined = 0b0000'0000,
        eSignal = 0b0000'0001,
        eSignalCall = 0b0000'0010
    };

    struct FiberAwaitable {
        /**
         *
         */
        FiberAwaitableBits mask;

        struct self_type {};

        /**
         *
         */
        union {
            /**
             *
             */
            await_signal_sub_type* signal;

            /**
             *
             */
            struct {
                ptr<void> self;

                bool (self_type::*call)();
            };
        };

    public:
        void reset() noexcept;

        [[nodiscard]] bool ready() const noexcept;

        [[nodiscard]] bool operator!() const noexcept;
    };
}
