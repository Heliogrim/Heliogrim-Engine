#pragma once

#include <Engine.Core/Module/CoreModule.hpp>
#include <Engine.Platform/NativeWindow.hpp>
#include <Engine.Event/GlobalEventEmitter.hpp>

#include "DragDrop/DragDropReceiver.hpp"
#include "DragDrop/DragDropSender.hpp"

namespace ember::engine::input::schedule {
    class InputTickStage;
}

namespace ember::engine {
    class Input :
        public core::CoreModule {
    public:
        friend class ::ember::engine::input::schedule::InputTickStage;

    public:
        using this_type = Input;

    public:
        Input(const non_owning_rptr<Engine> engine_) noexcept;

        ~Input() override;

    public:
        void setup() override;

        void start() override;

        void stop() override;

        void destroy() override;

    private:
        void tick();

    private:
        GlobalEventEmitter _emitter;

        _STD mutex _bufferMtx;
        Vector<_STD pair<event_type_id, uptr<Event>>> _buffered;

    public:
        [[nodiscard]] cref<GlobalEventEmitter> emitter() const noexcept;

        [[nodiscard]] ref<GlobalEventEmitter> emitter() noexcept;

    public:
        [[nodiscard]] bool tryBufferEvent(const event_type_id eventType_, mref<uptr<Event>> event_);

        void bufferEvent(const event_type_id eventType_, mref<uptr<Event>> event_);

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
