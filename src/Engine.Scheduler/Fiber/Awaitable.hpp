#pragma once

#include <atomic>
#include <concepts>
#include <functional>
#include <type_traits>

#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>

namespace ember::engine::scheduler::fiber {

    using await_signal_type = _STD atomic_flag;

    template <typename AwaitableType_>
    concept IsAwaitableSignal = _STD is_same_v<AwaitableType_, await_signal_type>;

    template <typename AwaitableType_>
    concept IsAwaitableSignalRet = requires(const AwaitableType_ obj) {
        { obj.await() } -> std::same_as<const ptr<await_signal_type>>;
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
            await_signal_type* signal;

            /**
             *
             */
            struct {
                ptr<void> self;

                bool (self_type::*call)();
            };

        };

    public:
        void reset() noexcept {
            mask = FiberAwaitableBits::eUndefined;
            self = nullptr;
            call = nullptr;
        }
    };
}
