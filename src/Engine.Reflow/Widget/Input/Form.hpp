#pragma once

#include <Engine.Event/EventEmitter.hpp>

#include "../Widget.hpp"
#include "../../Event/FormSubmitEvent.hpp"
#include "../../Children.hpp"

namespace hg::engine::reflow {
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
        SingleChildren _children;

    public:
        [[nodiscard]] const ptr<const SingleChildren> children() const override;

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
        void render(const ptr<ReflowCommandBuffer> cmd_) override;

    public:
        math::vec2 prefetchDesiredSize(cref<ReflowState> state_, float scale_) const override;

        math::vec2 computeDesiredSize(cref<ReflowPassState> passState_) const override;

        void applyLayout(ref<ReflowState> state_, mref<LayoutContext> ctx_) override;
    };
}
