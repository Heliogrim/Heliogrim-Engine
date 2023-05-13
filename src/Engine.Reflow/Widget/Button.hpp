#pragma once

#include <Engine.Event/EventEmitter.hpp>

#include "BoxPanel.hpp"
#include "../Attribute/StyleAttribute.hpp"
#include "../Style/ButtonStyle.hpp"

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

    protected:
        struct Attributes : public BoxPanel::Attributes {
            StyleAttribute<ButtonStyle> style;
        } _attr;

    public:
        [[nodiscard]] ref<Attributes> attributes() noexcept override;

    public:
        void render(cref<ReflowState> state_, const ptr<ReflowCommandBuffer> cmd_) override;

    private:
        StatelessEventEmitter<MouseEvent> _emitter;

    public:
        [[nodiscard]] decltype(_emitter)::handle_type addOnClick(mref<decltype(_emitter)::function_type> fnc_);

        void removeOnClick(decltype(_emitter)::handle_type handle_);

    public:
        EventResponse onMouseButtonDown(cref<MouseEvent> event_) override;
    };
}
