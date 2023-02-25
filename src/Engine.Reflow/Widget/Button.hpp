#pragma once

#include <Engine.Event/EventEmitter.hpp>

#include "HBox.hpp"

namespace ember::engine::reflow {
    class Button :
        public HBox {
    public:
        using this_type = Button;

    public:
        Button(mref<sptr<BoundStyleSheet>> style_);

        ~Button() override;

    public:
        [[nodiscard]] string getTag() const noexcept override;

    public:
        void render(const ptr<ReflowCommandBuffer> cmd_) override;

    private:
        StatelessEventEmitter<MouseEvent> _emitter;

    public:
        [[nodiscard]] decltype(_emitter)::handle_type addOnClick(mref<decltype(_emitter)::function_type> fnc_);

        void removeOnClick(decltype(_emitter)::handle_type handle_);

    public:
        EventResponse onMouseButtonDown(cref<MouseEvent> event_) override;
    };
}
