#pragma once

#include <Engine.Session/Session.hpp>
#include <Engine.Session/Window.hpp>

#include "DragDrop/DragDropReceiver.hpp"
#include "DragDrop/DragDropSender.hpp"

namespace ember::engine {

    class Input {
    public:
        using this_type = Input;

    public:
        Input(cref<sptr<Session>> session_ = Session::get()) noexcept;

        ~Input();

    public:
        void setup();

    private:
        void tidy();

    private:
        sptr<Session> _session;

    public:
        [[nodiscard]] sptr<Session> session() const noexcept;

    private:
        ptr<input::DragDropReceiver> _dragDropReceiver;
        ptr<input::DragDropSender> _dragDropSender;

    public:
        [[nodiscard]] const ptr<input::DragDropSender> dragDropSender() const noexcept;

    public:
        void captureWindow(const ptr<session::Window> window_);

        void releaseWindow(const ptr<session::Window> window_);
    };

}
