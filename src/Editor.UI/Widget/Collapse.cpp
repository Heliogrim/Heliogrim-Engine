#include "Collapse.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.Reflow/Style/BoundStyleSheet.hpp>
#include <Engine.Reflow/Style/StyleSheet.hpp>
#include <Engine.Reflow/Style/StyleCondition.hpp>

#include "../Style/Style.hpp"
#include "../Color/Dark.hpp"

using namespace ember::engine::reflow;
using namespace ember::editor::ui;
using namespace ember;

[[nodiscard]] static sptr<BoundStyleSheet> makeHeaderBoxStyle() {

    auto style {
        BoundStyleSheet::make(
            StyleSheet {
                .minWidth { true, ReflowUnit { ReflowUnitType::eAuto, 0.F } },
                .width { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
                .maxWidth { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
                .minHeight { true, ReflowUnit { ReflowUnitType::eAbsolute, 20.F } },
                .height { true, ReflowUnit { ReflowUnitType::eAbsolute, 20.F } },
                .maxHeight = { true, ReflowUnit { ReflowUnitType::eAbsolute, 20.F } },
                .wrap { true, ReflowWrap::eNoWrap },
                .padding { true, Padding { 0.F, 4.F } },
                .reflowSpacing { true, ReflowSpacing::eSpaceBetween },
                .color { true, color::Dark::backgroundDefault }
            }
        )
    };

    return style;
}

CollapseHeader::CollapseHeader(ptr<Collapse> parent_) :
    HBox(makeHeaderBoxStyle()),
    _parent(parent_) {}

CollapseHeader::~CollapseHeader() = default;

void CollapseHeader::setup() {

    _indicator = make_sptr<Text>(BoundStyleSheet::make(Style::get()->getStyleSheet(Style::TitleSmallKey)));
    _indicator->setText(R"(>)");
    this->addChild(_indicator);

    _content = make_sptr<Text>(BoundStyleSheet::make(Style::get()->getStyleSheet(Style::TitleSmallKey)));
    static_cast<ptr<Text>>(_content.get())->setText(R"(Collapse)");
    this->addChild(_content);
}

void CollapseHeader::setTitle(cref<string> title_) {
    static_cast<ptr<Text>>(_content.get())->setText(title_);
}

engine::reflow::EventResponse CollapseHeader::onMouseButtonDown(cref<engine::reflow::MouseEvent> event_) {

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

[[nodiscard]] static sptr<BoundStyleSheet> makeCollapseBoxStyle() {

    auto style {
        BoundStyleSheet::make(
            StyleSheet {
                .minWidth { true, ReflowUnit { ReflowUnitType::eAuto, 0.F } },
                .width { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
                .maxWidth { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
                .minHeight { true, ReflowUnit { ReflowUnitType::eAbsolute, 20.F } },
                .height { true, ReflowUnit { ReflowUnitType::eAbsolute, 20.F } },
                .maxHeight = { true, ReflowUnit { ReflowUnitType::eAbsolute, 20.F } },
                .wrap { true, ReflowWrap::eNoWrap },
                .colGap { true, ReflowUnit { ReflowUnitType::eAuto, 0.F } },
                .reflowSpacing { true, ReflowSpacing::eStart },
                .color { true, color::Dark::backgroundDefault }
            }
        )
    };

    return style;
}

Collapse::Collapse() :
    VBox(makeCollapseBoxStyle()),
    _collapsed(true) {}

Collapse::~Collapse() = default;

void Collapse::setup() {

    _header = make_sptr<CollapseHeader>(this);
    _header->setup();
    this->addChild(_header);

    _content = make_sptr<VBox>(BoundStyleSheet::make(Style::get()->getStyleSheet(Style::AdoptFlexBoxKey)));
    _content->style()->minHeight = ReflowUnit { ReflowUnitType::eAuto, 0.F };
    _content->style()->height = ReflowUnit { ReflowUnitType::eAuto, 0.F };
    _content->style()->maxHeight = ReflowUnit { ReflowUnitType::eAuto, 0.F };
    _content->style()->wrap = ReflowWrap::eNoWrap;
    _content->style()->colGap = ReflowUnit { ReflowUnitType::eAbsolute, 4.F };
    _content->style()->color = color::Dark::backgroundDefault;
    this->addChild(_content);

    collapse();
}

void Collapse::collapse() {
    _collapsed = true;
    markAsPending();

    _style->height = ReflowUnit { ReflowUnitType::eAbsolute, 20.F };
    _style->maxHeight = ReflowUnit { ReflowUnitType::eAbsolute, 20.F };

    if (_content) {
        _content->state().unwrap &= ~(static_cast<u8>(WidgetStateFlagBits::eVisible));
        _content->markAsPending();
    }
}

void Collapse::expand() {
    _collapsed = false;
    markAsPending();

    _style->height = ReflowUnit { ReflowUnitType::eAuto, 0.F };
    _style->maxHeight = ReflowUnit { ReflowUnitType::eAuto, 0.F };

    if (_content) {
        _content->state() |= WidgetStateFlagBits::eVisible;
        _content->markAsPending();
    }
}

cref<sptr<CollapseHeader>> Collapse::getHeader() noexcept {
    return _header;
}

cref<sptr<Box>> Collapse::getContent() noexcept {
    return _content;
}

string Collapse::getTag() const noexcept {
    return _STD format(R"(Collapse <{:#x}>)", reinterpret_cast<u64>(this));
}

bool Collapse::willChangeLayout(
    cref<math::vec2> space_,
    cref<engine::reflow::StyleKeyStack> styleStack_
) const noexcept {
    return VBox::willChangeLayout(space_, styleStack_);
}

void Collapse::flow(
    cref<engine::reflow::FlowContext> ctx_,
    cref<math::vec2> space_,
    cref<math::vec2> limit_,
    ref<engine::reflow::StyleKeyStack> styleStack_
) {
    VBox::flow(ctx_, space_, limit_, styleStack_);
}

void Collapse::shift(cref<engine::reflow::FlowContext> ctx_, cref<math::vec2> offset_) {
    VBox::shift(ctx_, offset_);
}
