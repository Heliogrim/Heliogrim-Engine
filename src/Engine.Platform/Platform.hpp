#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Scheduler/Task/SignaledQueue.hpp>
#include <Engine.Common/String.hpp>
#include <Engine.Common/Math/Extent.hpp>
#include <Engine.Common/Concurrent/Future.hpp>

namespace ember::engine::platform {
    class NativeWindow;
}

namespace ember::engine {
    class __declspec(novtable) Platform {
    public:
        using this_type = Platform;

    public:
        Platform() noexcept;

        Platform(cref<this_type>) = delete;

        Platform(mref<this_type>) noexcept = delete;

        virtual ~Platform();

    public:
        ref<this_type> operator=(cref<this_type>) = delete;

        ref<this_type> operator=(mref<this_type>) noexcept = delete;

    public:
        virtual void tidy() = 0;

        virtual void setup() = 0;

    public:
        [[nodiscard]] virtual ref<scheduler::task::SignaledQueue> platformQueue() const noexcept = 0;

    public:
        [[nodiscard]] virtual ::ember::concurrent::future<uptr<platform::NativeWindow>> makeNativeWindow(
            const string_view title_, cref<math::iExtent2D> extent_
        ) = 0;

        [[nodiscard]] virtual ::ember::concurrent::future<bool> destroyNativeWindow(
            mref<uptr<platform::NativeWindow>> window_
        ) = 0;
    };
}
