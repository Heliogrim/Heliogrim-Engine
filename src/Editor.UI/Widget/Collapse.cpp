#include "Collapse.hpp"

#include <format>
#include <Engine.Common/Make.hpp>
#include <Engine.Reflow/Algorithm/Flex.hpp>
#include <Engine.Reflow/Algorithm/FlexState.hpp>
#include <Engine.Reflow/Layout/Style.hpp>

#include "../Color/Dark.hpp"
#include "Editor.UI/Theme/Theme.hpp"

using namespace hg::engine::reflow;
using namespace hg::editor::ui;
using namespace hg;

CollapseHeader::CollapseHeader(ptr<Collapse> parent_) :
    HorizontalPanel(),
    _parent(parent_) {
    /**/
    attr.width.setValue({ ReflowUnitType::eRelative, 1.F });
    attr.maxWidth.setValue({ ReflowUnitType::eRelative, 1.F });
    attr.minHeight.setValue({ ReflowUnitType::eAbsolute, 20.F });
    attr.height.setValue({ ReflowUnitType::eAbsolute, 20.F });
    attr.maxHeight.setValue({ ReflowUnitType::eAbsolute, 20.F });
    attr.padding.setValue(Padding { 0.F, 4.F });
    attr.justify.setValue(ReflowSpacing::eSpaceBetween);
}

CollapseHeader::~CollapseHeader() = default;

void CollapseHeader::setup() {

    const auto* const theme = Theme::get();

    _indicator = make_sptr<Text>();
    theme->applyLabel(_indicator);
    _indicator->setText(R"(>)");
    this->addChild(_indicator);

    _content = make_sptr<Text>();
    theme->applyLabel(_STD static_pointer_cast<Text, Widget>(_content));
    static_cast<ptr<Text>>(_content.get())->setText(R"(Collapse)");
    this->addChild(_content);
}

void CollapseHeader::setTitle(cref<string> title_) {
    static_cast<ptr<Text>>(_content.get())->setText(title_);
}

EventResponse CollapseHeader::onMouseButtonDown(cref<MouseEvent> event_) {

    if (event_._button != 0x1) {
        return EventResponse::eHandled;
    }

    /**/

    if (_parent->attr.collapsed.getValue()) {
        _parent->expand();
    } else {
        _parent->collapse();
    }

    return EventResponse::eConsumed;
}

string CollapseHeader::getTag() const noexcept {
    return _STD format(R"(CollapseHeader <{:#x}>)", reinterpret_cast<u64>(this));
}

Collapse::Collapse() :
    Widget(),
    attr(
        Attributes {
            .minWidth = { this, { ReflowUnitType::eRelative, 1.F } },
            .maxWidth = { this, { ReflowUnitType::eRelative, 1.F } },
            .minHeight = { this, { ReflowUnitType::eAuto } },
            .maxHeight = { this, { ReflowUnitType::eAuto } },
            .collapsed = { this, true }
        }
    ),
    _children() {}

Collapse::~Collapse() = default;

void Collapse::setup() {

    auto header = make_sptr<CollapseHeader>(this);
    header->setup();
    header->setParent(shared_from_this());
    _children.setChild<0>(_STD move(header));

    collapse();
}

void Collapse::collapse() {
    attr.collapsed.setValue(true);
    markAsPending();

    const auto body = _children.getChild<1>();
    if (body) {
        body->state().unset(WidgetStateFlagBits::eVisible);
        body->markAsPending();
    }
}

void Collapse::expand() {
    attr.collapsed.setValue(false);
    markAsPending();

    const auto body = _children.getChild<1>();
    if (body) {
        body->state().set(WidgetStateFlagBits::eVisible);
        body->markAsPending();
    }
}

const ptr<const engine::reflow::Children> Collapse::children() const {
    return &_children;
}

sptr<CollapseHeader> Collapse::getHeader() noexcept {
    return _STD static_pointer_cast<CollapseHeader, Widget>(_children.getChild<0>());
}

sptr<Widget> Collapse::getContent() const noexcept {
    return _children.getChild<1>();
}

void Collapse::setContent(cref<sptr<Widget>> widget_) {
    widget_->setParent(shared_from_this());
    const auto prev = _children.setChild<1>(widget_);
    prev->setParent(nullptr);
}

string Collapse::getTag() const noexcept {
    return _STD format(R"(Collapse <{:#x}>)", reinterpret_cast<u64>(this));
}

void Collapse::render(const ptr<engine::reflow::ReflowCommandBuffer> cmd_) {

    math::fExtent2D scissor = math::fExtent2D {
        _layoutState.layoutSize.x, _layoutState.layoutSize.y,
        _layoutState.layoutOffset.x, _layoutState.layoutOffset.y
    };

    cmd_->pushIntersectScissor(scissor);

    /**/

    for (const auto& child : _children) {

        if (child->state().isVisible() && not cmd_->scissorCull(
            child->layoutState().layoutOffset,
            child->layoutState().layoutSize
        )) {
            child->render(cmd_);
        }

    }

    /**/

    cmd_->popScissor();
}

math::vec2 Collapse::prefetchDesiredSize(cref<ReflowState> state_, float scale_) const {

    const auto headerSize = _children.getChild<0>()->getDesiredSize();
    const auto bodySize = _children.getChild<1>()->getDesiredSize();

    /**/

    math::vec2 desired = headerSize;
    if (not attr.collapsed.getValue()) {
        desired = math::vec2 { MAX(headerSize.x, bodySize.x), headerSize.y + bodySize.y };
    }

    /**/

    return layout::clampSizeAbs(attr, desired);
}

math::vec2 Collapse::computeDesiredSize(cref<ReflowPassState> passState_) const {
    const math::vec2 desired = getDesiredSize();
    return layout::clampSize(attr, passState_.layoutSize, desired);
}

void Collapse::applyLayout(ref<ReflowState> state_, mref<LayoutContext> ctx_) {

    algorithm::FlexState flexState {};
    flexState.box.preservedSize = ctx_.localSize;
    flexState.box.maxSize = ctx_.localSize;
    flexState.box.orientation = algorithm::FlexLineOrientation::eVertical;
    flexState.box.justify = ReflowSpacing::eStart;
    flexState.box.align = ReflowAlignment::eCenter;
    flexState.box.wrap = false;

    algorithm::solve(flexState, state_, children());

    for (const auto& flexLine : flexState.lines) {
        for (const auto& flexItem : flexLine.items) {

            const auto dummy = flexItem.widget.lock();
            const auto widgetState = state_.getStateOf(_STD static_pointer_cast<Widget, void>(dummy));

            widgetState->layoutOffset = flexItem.offset + ctx_.localOffset;
            widgetState->layoutSize = flexItem.flexSize;
        }
    }

    if (attr.collapsed.getValue()) {
        _children.getChild<1>()->layoutState().layoutSize = math::vec2 { 0.F };
    }
}

bool Collapse::willChangeLayout(
    cref<math::vec2> space_
) const noexcept {
    return Widget::willChangeLayout(space_);
}
