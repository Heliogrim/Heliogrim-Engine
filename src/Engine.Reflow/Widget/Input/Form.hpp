#pragma once

#include <Engine.Event/EventEmitter.hpp>

#include "../Widget.hpp"
#include "../../Event/FormSubmitEvent.hpp"
#include "../../Children.hpp"

namespace ember::engine::reflow {
    class Form :
        public Widget {
    public:
        using this_type = Form;

    public:
        Form();

        ~Form() override;

    public:
        [[nodiscard]] string getTag() const noexcept override;

    private:
        Children _children;

    public:
        void setContent(cref<sptr<Widget>> content_);

        [[nodiscard]] sptr<Widget> getContent() const noexcept;

    public:
        void submitForm();

        void clearForm();

    private:
        StatelessEventEmitter<FormSubmitEvent> _submitEmitter;

    public:
        [[nodiscard]] decltype(_submitEmitter)::handle_type onSubmit(
            mref<decltype(_submitEmitter)::function_type> fnc_
        );

    public:
        [[nodiscard]] const ptr<const Children> children() const override;

    public:
        void render(const ptr<ReflowCommandBuffer> cmd_) override;

        void flow(
            cref<FlowContext> ctx_,
            cref<math::vec2> space_,
            cref<math::vec2> limit_,
            ref<StyleKeyStack> styleStack_
        ) override;

        void shift(cref<FlowContext> ctx_, cref<math::vec2> offset_) override;

    public:
        [[nodiscard]] math::vec2 outerSize() const noexcept override;

        [[nodiscard]] math::vec2 innerSize() const noexcept override;

        [[nodiscard]] math::vec2 screenOffset() const noexcept override;
    };
}
