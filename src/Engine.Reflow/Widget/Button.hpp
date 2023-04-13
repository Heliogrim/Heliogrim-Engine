#pragma once

#include <Engine.Event/EventEmitter.hpp>

#include "BoxPanel.hpp"

namespace hg::engine::reflow {
    class Button :
        public BoxPanel {
    public:
        using this_type = Button;

    public:
        Button();

        ~Button() override;

    public:
        [[nodiscard]] string getTag() const noexcept override;

    private:
        StatelessEventEmitter<MouseEvent> _emitter;

    public:
        [[nodiscard]] decltype(_emitter)::handle_type addOnClick(mref<decltype(_emitter)::function_type> fnc_);

        void removeOnClick(decltype(_emitter)::handle_type handle_);

    public:
        EventResponse onMouseButtonDown(cref<MouseEvent> event_) override;
    };
}
