#include "InputVec.hpp"

using namespace hg::editor::ui;
using namespace hg::engine::reflow;
using namespace hg;

sptr<BoundStyleSheet> __inputvec::makeInputBoxStyle() {

    auto style {
        BoundStyleSheet::make(
            StyleSheet {
                .minWidth { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
                .width { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
                .maxWidth { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
                .minHeight { true, ReflowUnit { ReflowUnitType::eAbsolute, 20.F } },
                .height { true, ReflowUnit { ReflowUnitType::eAbsolute, 20.F } },
                .maxHeight { true, ReflowUnit { ReflowUnitType::eAbsolute, 20.F } },
                .wrap { true, ReflowWrap::eNoWrap },
                .padding { true, Padding { 4.F, 2.F } },
                .margin { true, Margin { 0.F } },
                .reflowShrink { true, 1.F },
                .reflowGrow { true, 0.F },
                .borderRadius = { true, BorderRadius { 4.F } },
                .color { false, color::Dark::backgroundInnerField }
            }
        )

    };

    style->pushStyle(
        {
            Style::key_type::from("InputType::Focused"),
            style::isFocused,
            make_sptr<StyleSheet>(
                StyleSheet {
                    .color { true, color::Dark::backgroundInnerFieldDarken }
                }
            )
        }
    );

    return style;
}

sptr<engine::reflow::BoundStyleSheet> __inputvec::makeInputTextStyle() {
    auto* font { getDefaultFont() };

    auto style {
        BoundStyleSheet::make(
            StyleSheet {
                .minWidth { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
                .width { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
                .maxWidth { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
                .minHeight { true, ReflowUnit { ReflowUnitType::eAbsolute, 16.F } },
                .height { true, ReflowUnit { ReflowUnitType::eAbsolute, 16.F } },
                .maxHeight { true, ReflowUnit { ReflowUnitType::eAbsolute, 16.F } },
                .color { false, color::Dark::grey },
                .font { true, font },
                .fontSize { true, 16.F },
                .textAlign { true, TextAlign::eLeftMiddle },
                .textEllipse { true, 1ui32 }
            }
        )
    };

    style->pushStyle(
        {
            Style::key_type::from("InputType::Focused"),
            style::isNever,
            make_sptr<StyleSheet>(
                StyleSheet {
                    .color { true, color::Dark::white }
                }
            )
        }
    );

    return style;
}
