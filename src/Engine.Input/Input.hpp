#pragma once

#include <Engine.Core/CoreModule.hpp>
#include <Engine.Platform/NativeWindow.hpp>

#include "DragDrop/DragDropReceiver.hpp"
#include "DragDrop/DragDropSender.hpp"

namespace ember::engine {
    class Input :
        public core::CoreModule {
    public:
        using this_type = Input;

    public:
        Input(const non_owning_rptr<Engine> engine_) noexcept;

        ~Input() override;

    public:
        void setup() override;

        void schedule() override;

        void desync() override;

        void destroy() override;

    private:
        ptr<input::DragDropReceiver> _dragDropReceiver;
        ptr<input::DragDropSender> _dragDropSender;

    public:
        [[nodiscard]] const ptr<input::DragDropSender> dragDropSender() const noexcept;

    public:
        void captureWindow(const non_owning_rptr<platform::NativeWindow> nativeWindow_);

        void releaseWindow(const non_owning_rptr<platform::NativeWindow> nativeWindow_);
    };
}
