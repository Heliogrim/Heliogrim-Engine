#include "Style.hpp"

#include <Engine.Common/Make.hpp>

#include "Editor.UI/Color/Dark.hpp"
#include "Engine.GFX.Glow.UI/TestUI.hpp"

using namespace ember::editor::ui;
using namespace ember::engine::reflow;
using namespace ember;

uptr<Style> Style::_instance = nullptr;

const ptr<Style::this_type> Style::get() {
    return _instance.get();
}

const ptr<Style::this_type> Style::make() {

    if (not _instance) {
        _instance = uptr<Style>(new Style());
        _instance->setup();
    }

    return _instance.get();
}

void Style::destroy() {
    _instance.reset();
}

Style::Style() = default;

Style::~Style() noexcept = default;

void Style::setup() {

    auto style { make_sptr<StyleSheet>() };

    /* Default */
    _styles.insert_or_assign(Style::DefaultKey, _STD move(style));

    /* Neutral */
    style = make_sptr<StyleSheet>();

    style->color = engine::color { 0.F, 0.F, 0.F, 0.F };

    _styles.insert_or_assign(Style::NeutralKey, _STD move(style));

    /* Scroll Box */
    style = make_sptr<StyleSheet>();

    style->width = ReflowUnit { ReflowUnitType::eRelative, 1.F };
    style->height = ReflowUnit { ReflowUnitType::eRelative, 1.F };
    style->maxHeight = ReflowUnit { ReflowUnitType::eRelative, 1.F };

    style->padding = Padding { 6.F };

    style->reflowSpacing = ReflowSpacing::eStart;
    style->reflowShrink = 1.F;
    style->reflowGrow = 1.F;

    style->color = color::Dark::backgroundDefault;

    _styles.insert_or_assign(Style::ScrollBoxKey, _STD move(style));

    /* Adopt Flex Box */
    style = make_sptr<StyleSheet>();

    style->width = ReflowUnit { ReflowUnitType::eRelative, 1.F };
    style->maxWidth = ReflowUnit { ReflowUnitType::eRelative, 1.F };

    style->height = ReflowUnit { ReflowUnitType::eRelative, 1.F };
    style->maxHeight = ReflowUnit { ReflowUnitType::eRelative, 1.F };

    style->reflowShrink = 1.F;
    style->reflowGrow = 1.F;

    style->color = color::Dark::backgroundDefault;

    _styles.insert_or_assign(Style::AdoptFlexBoxKey, _STD move(style));

    /* Button */
    style = make_sptr<StyleSheet>();

    style->color = color::Dark::backgroundDefault;
    style->padding = Padding { 4.F };
    style->reflowSpacing = ReflowSpacing::eSpaceAround;

    _styles.insert_or_assign(Style::ButtonKey, _STD move(style));

    /* Text Button */
    style = make_sptr<StyleSheet>();

    style->color = color::Dark::backgroundDefault;
    style->padding = Padding { 4.F, 2.F };
    style->reflowSpacing = ReflowSpacing::eSpaceAround;

    _styles.insert_or_assign(Style::TextButtonKey, _STD move(style));

    /* Icon 32 */
    style = make_sptr<StyleSheet>();

    style->minWidth = ReflowUnit { ReflowUnitType::eAbsolute, 32.F };
    style->width = ReflowUnit { ReflowUnitType::eAbsolute, 32.F };
    style->maxWidth = ReflowUnit { ReflowUnitType::eAbsolute, 32.F };

    style->minHeight = ReflowUnit { ReflowUnitType::eAbsolute, 32.F };
    style->height = ReflowUnit { ReflowUnitType::eAbsolute, 32.F };
    style->maxHeight = ReflowUnit { ReflowUnitType::eAbsolute, 32.F };

    style->color = color::Dark::white;
    style->padding = Padding { 0.F };

    _styles.insert_or_assign(Style::Icon32Key, _STD move(style));

    /* Icon 72 */
    style = make_sptr<StyleSheet>();

    style->minWidth = ReflowUnit { ReflowUnitType::eAbsolute, 72.F };
    style->width = ReflowUnit { ReflowUnitType::eAbsolute, 72.F };
    style->maxWidth = ReflowUnit { ReflowUnitType::eAbsolute, 72.F };

    style->minHeight = ReflowUnit { ReflowUnitType::eAbsolute, 72.F };
    style->height = ReflowUnit { ReflowUnitType::eAbsolute, 72.F };
    style->maxHeight = ReflowUnit { ReflowUnitType::eAbsolute, 72.F };

    style->color = color::Dark::white;
    style->padding = Padding { 0.F };

    _styles.insert_or_assign(Style::Icon72Key, _STD move(style));

    /* Icon 96 */
    style = make_sptr<StyleSheet>();

    style->minWidth = ReflowUnit { ReflowUnitType::eAbsolute, 96.F };
    style->width = ReflowUnit { ReflowUnitType::eAbsolute, 96.F };
    style->maxWidth = ReflowUnit { ReflowUnitType::eAbsolute, 96.F };

    style->minHeight = ReflowUnit { ReflowUnitType::eAbsolute, 96.F };
    style->height = ReflowUnit { ReflowUnitType::eAbsolute, 96.F };
    style->maxHeight = ReflowUnit { ReflowUnitType::eAbsolute, 96.F };

    style->color = color::Dark::white;
    style->padding = Padding { 0.F };

    _styles.insert_or_assign(Style::Icon96Key, _STD move(style));

    /* Icon 128 */
    style = make_sptr<StyleSheet>();

    style->minWidth = ReflowUnit { ReflowUnitType::eAbsolute, 128.F };
    style->width = ReflowUnit { ReflowUnitType::eAbsolute, 128.F };
    style->maxWidth = ReflowUnit { ReflowUnitType::eAbsolute, 128.F };

    style->minHeight = ReflowUnit { ReflowUnitType::eAbsolute, 128.F };
    style->height = ReflowUnit { ReflowUnitType::eAbsolute, 128.F };
    style->maxHeight = ReflowUnit { ReflowUnitType::eAbsolute, 128.F };

    style->color = color::Dark::white;
    style->padding = Padding { 0.F };

    _styles.insert_or_assign(Style::Icon128Key, _STD move(style));

    /**/
    auto* font { getDefaultFont() };
    /**/

    /* Title */

    style = make_sptr<StyleSheet>(
        StyleSheet {
            .color = { true, color::Dark::grey },
            .font = { true, font },
            .fontSize = { true, 16.F },
            .textAlign = { true, TextAlign::eCenterMiddle },
            .textEllipse = { true, 0ui32 }
        }
    );

    _styles.insert_or_assign(Style::TitleKey, _STD move(style));

    /* Title Small */

    style = make_sptr<StyleSheet>(
        StyleSheet {
            .color = { true, color::Dark::grey },
            .font = { true, font },
            .fontSize = { true, 12.F },
            .textAlign = { true, TextAlign::eCenterMiddle },
            .textEllipse = { true, 0ui32 }
        }
    );

    _styles.insert_or_assign(Style::TitleSmallKey, _STD move(style));

    /* Button::Raised */

    style = make_sptr<StyleSheet>(
        StyleSheet {
            .color = { true, color::Dark::raisedColor },
        }
    );

    _styles.insert_or_assign(Style::ButtonRaisedKey, _STD move(style));

    /* Title::Raised */

    style = make_sptr<StyleSheet>(
        StyleSheet {
            .color = { true, color::Dark::white },
        }
    );

    _styles.insert_or_assign(Style::TitleRaisedKey, _STD move(style));

    /* Breadcrumb */

    style = make_sptr<StyleSheet>(
        StyleSheet {
            .maxWidth = { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
            .minHeight = { true, ReflowUnit { ReflowUnitType::eAbsolute, 20.F } },
            .maxHeight = { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
            .wrap = { true, ReflowWrap::eNoWrap },
            .rowGap = { true, ReflowUnit { ReflowUnitType::eAbsolute, 2.F } },
            .padding = { true, Padding { 4.F, 2.F } },
            .color = { true, color::Dark::backgroundDefault }
        }
    );

    _styles.insert_or_assign(Style::BreadcrumbKey, _STD move(style));

    /* Nav Bar */

    style = make_sptr<StyleSheet>(
        StyleSheet {
            .width = { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
            .maxWidth = { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
            .minHeight = { true, ReflowUnit { ReflowUnitType::eAbsolute, 20.F } },
            .maxHeight = { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
            .wrap = { true, ReflowWrap::eNoWrap },
            .padding = { true, Padding { 4.F, 0.F } },
            .reflowSpacing { true, ReflowSpacing::eSpaceBetween },
            .reflowShrink { true, 0.F },
            .color = { true, color::Dark::backgroundDefault }
        }
    );

    _styles.insert_or_assign(Style::NavBarKey, _STD move(style));

}

sptr<engine::reflow::StyleSheet> Style::getStyleSheet(cref<key_type> key_) const noexcept {

    const auto where { _styles.find(key_) };
    if (where != _styles.end()) {
        return where->second;
    }

    return nullptr;
}
