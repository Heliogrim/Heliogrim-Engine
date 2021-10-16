#pragma once

#include <condition_variable>
#include <functional>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Concurrent/Collection/RingBuffer.hpp>

namespace ember::engine::scheduler::task {

    class SignaledQueue final {
    public:
        using value_type = SignaledQueue;
        using reference_type = ref<value_type>;
        using const_reference_type = cref<value_type>;

    public:
        SignaledQueue() = default;

        ~SignaledQueue() = default;

    public:
        void signal();

        void finalize();

        [[nodiscard]] bool finalized() const noexcept;

        [[nodiscard]] bool empty() const noexcept;

        bool try_push(mref<_STD function<void()>> fnc_);

    private:
        bool pop(OUT ref<_STD function<void()>> fnc_);

    public:
        bool waitPop(OUT ref<_STD function<void()>> fnc_);

    private:
        _STD mutex _mtx;
        _STD condition_variable _sig;
        _STD atomic_flag _mode;

    private:
        concurrent::RingBuffer<std::function<void()>> _buffer;
    };

}
