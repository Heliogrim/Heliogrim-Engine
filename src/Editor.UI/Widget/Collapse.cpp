#include "Collapse.hpp"

#include <Engine.Common/Make.hpp>

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

    if (_parent->_collapsed) {
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
    VerticalPanel(),
    _collapsed(true) {
    /**/
    attr.width.setValue({ ReflowUnitType::eRelative, 1.F });
    attr.maxWidth.setValue({ ReflowUnitType::eRelative, 1.F });
    attr.minHeight.setValue({ ReflowUnitType::eAbsolute, 20.F });
    attr.height.setValue({ ReflowUnitType::eAbsolute, 20.F });
    attr.maxHeight.setValue({ ReflowUnitType::eAbsolute, 20.F });
}

Collapse::~Collapse() = default;

void Collapse::setup() {

    _header = make_sptr<CollapseHeader>(this);
    _header->setup();
    this->addChild(_header);

    _content = make_sptr<VerticalPanel>();
    _content->attr.minHeight.setValue({ ReflowUnitType::eAuto, 0.F });
    _content->attr.height.setValue({ ReflowUnitType::eAuto, 0.F });
    _content->attr.maxHeight.setValue({ ReflowUnitType::eAuto, 0.F });
    _content->attr.colGap.setValue(4.F);

    this->addChild(_content);

    collapse();
}

void Collapse::collapse() {
    _collapsed = true;
    markAsPending();

    attr.height.setValue({ ReflowUnitType::eAbsolute, 20.F });
    attr.maxHeight.setValue({ ReflowUnitType::eAbsolute, 20.F });

    if (_content) {
        _content->state().unwrap &= ~(static_cast<u8>(WidgetStateFlagBits::eVisible));
        _content->markAsPending();
    }
}

void Collapse::expand() {
    _collapsed = false;
    markAsPending();

    attr.height.setValue({ ReflowUnitType::eAuto, 0.F });
    attr.maxHeight.setValue({ ReflowUnitType::eAuto, 0.F });

    if (_content) {
        _content->state() |= WidgetStateFlagBits::eVisible;
        _content->markAsPending();
    }
}

cref<sptr<CollapseHeader>> Collapse::getHeader() noexcept {
    return _header;
}

cref<sptr<VerticalPanel>> Collapse::getContent() noexcept {
    return _content;
}

string Collapse::getTag() const noexcept {
    return _STD format(R"(Collapse <{:#x}>)", reinterpret_cast<u64>(this));
}

math::vec2 Collapse::prefetchDesiredSize(cref<ReflowState> state_, float scale_) const {
    return VerticalPanel::prefetchDesiredSize(state_, scale_);
}

math::vec2 Collapse::computeDesiredSize(cref<ReflowPassState> passState_) const {
    return VerticalPanel::computeDesiredSize(passState_);
}

void Collapse::applyLayout(ref<ReflowState> state_, mref<LayoutContext> ctx_) {
    VerticalPanel::applyLayout(state_, _STD move(ctx_));
}

bool Collapse::willChangeLayout(
    cref<math::vec2> space_
) const noexcept {
    return VerticalPanel::willChangeLayout(space_);
}
