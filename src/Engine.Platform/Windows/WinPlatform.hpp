#pragma once

#include <Engine.Scheduler/Thread/Thread.hpp>

#include "../Platform.hpp"

namespace ember::engine {
    class WinPlatform final :
        public Platform {
    public:
        using this_type = WinPlatform;

    public:
        WinPlatform();

        ~WinPlatform() override;

    private:
        void setupThread();

    public:
        void tidy() override;

        void setup() override;

    private:
        _STD thread _thread;
        scheduler::task::SignaledQueue _platformQueue;

    public:
        [[nodiscard]] ref<scheduler::task::SignaledQueue> platformQueue() const noexcept override;

        [[nodiscard]] ember::concurrent::future<uptr<platform::NativeWindow>> makeNativeWindow(
            const string_view title_, cref<math::iExtent2D> extent_
        ) override;

        [[nodiscard]] ember::concurrent::future<bool> destroyNativeWindow(
            mref<uptr<platform::NativeWindow>> window_
        ) override;

    private:
        void processInternal();
    };
}
