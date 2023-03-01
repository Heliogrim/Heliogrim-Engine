#pragma once

#include <Engine.Reflow/Widget/Text.hpp>
#include <Engine.Reflow/Widget/HBox.hpp>
#include <Engine.Reflow/Widget/VBox.hpp>

namespace hg::editor::ui {
    class Collapse;
}

namespace hg::editor::ui {
    class CollapseHeader :
        public engine::reflow::HBox {
    public:
        friend class Collapse;

    public:
        using this_type = CollapseHeader;

    public:
        CollapseHeader(ptr<Collapse> parent_);

        ~CollapseHeader() override;

    private:
        void setup();

    private:
        ptr<Collapse> _parent;

    private:
        sptr<engine::reflow::Text> _indicator;
        sptr<engine::reflow::Widget> _content;

    public:
        void setTitle(cref<string> title_);

    public:
        engine::reflow::EventResponse onMouseButtonDown(cref<engine::reflow::MouseEvent> event_) override;

    public:
        [[nodiscard]] string getTag() const noexcept override;
    };

    class Collapse :
        public engine::reflow::VBox {
    public:
        friend class CollapseHeader;

    public:
        using this_type = Collapse;

    public:
        Collapse();

        ~Collapse() override;

    public:
        void setup();

    private:
        bool _collapsed;

    public:
        void collapse();

        void expand();

    private:
        sptr<CollapseHeader> _header;
        sptr<Box> _content;

    public:
        [[nodiscard]] cref<sptr<CollapseHeader>> getHeader() noexcept;

        [[nodiscard]] cref<sptr<Box>> getContent() noexcept;

    public:
        [[nodiscard]] string getTag() const noexcept override;

    public:
        [[nodiscard]] bool willChangeLayout(
            cref<math::vec2> space_,
            cref<engine::reflow::StyleKeyStack> styleStack_
        ) const noexcept override;

        void flow(
            cref<engine::reflow::FlowContext> ctx_,
            cref<math::vec2> space_,
            cref<math::vec2> limit_,
            ref<engine::reflow::StyleKeyStack> styleStack_
        ) override;

        void shift(cref<engine::reflow::FlowContext> ctx_, cref<math::vec2> offset_) override;
    };
}
