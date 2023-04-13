#pragma once

#include <Engine.Reflow/Widget/Text.hpp>
#include <Engine.Reflow/Widget/HorizontalPanel.hpp>
#include <Engine.Reflow/Widget/VerticalPanel.hpp>

namespace hg::editor::ui {
    class Collapse;
}

namespace hg::editor::ui {
    class CollapseHeader :
        public engine::reflow::HorizontalPanel {
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
        public engine::reflow::VerticalPanel {
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
        sptr<VerticalPanel> _content;

    public:
        [[nodiscard]] cref<sptr<CollapseHeader>> getHeader() noexcept;

        [[nodiscard]] cref<sptr<VerticalPanel>> getContent() noexcept;

    public:
        [[nodiscard]] string getTag() const noexcept override;

    public:
        math::vec2 prefetchDesiredSize(cref<engine::reflow::ReflowState> state_, float scale_) const override;

        math::vec2 computeDesiredSize(cref<engine::reflow::ReflowPassState> passState_) const override;

        void applyLayout(ref<engine::reflow::ReflowState> state_, mref<engine::reflow::LayoutContext> ctx_) override;

    public:
        [[nodiscard]] bool willChangeLayout(
            cref<math::vec2> space_
        ) const noexcept override;
    };
}
