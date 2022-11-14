#pragma once

#include "Widget.hpp"

namespace ember::engine::reflow {

    class Overlay :
        public Widget {
    public:
        using this_type = Overlay;

    public:
        Overlay();

        ~Overlay() override;

    public:
        [[nodiscard]] string getTag() const noexcept override;

    private:
        sptr<Widget> _content;

    public:
        [[nodiscard]] sptr<Widget> getContent() const noexcept;

        void setContent(cref<sptr<Widget>> content_);

    public:
        void render(const ptr<ReflowCommandBuffer> cmd_) override;

        void flow(cref<FlowContext> ctx_, cref<math::vec2> space_, cref<math::vec2> limit_, ref<StyleKeyStack> styleStack_) override;

        void shift(cref<FlowContext> ctx_, cref<math::vec2> offset_) override;

    public:
        [[nodiscard]] const ptr<const Children> children() const override;

    public:
        [[nodiscard]] ReflowPosition position() const noexcept override;

        [[nodiscard]] math::vec2 outerSize() const noexcept override;

        [[nodiscard]] math::vec2 innerSize() const noexcept override;

        [[nodiscard]] math::vec2 screenOffset() const noexcept override;
    };

}
