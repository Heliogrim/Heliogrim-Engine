#include "Breadcrumb.hpp"

#include "../Style/Style.hpp"
#include <Engine.Reflow/Style/BoundStyleSheet.hpp>
#include <Engine.Reflow/Style/StyleCondition.hpp>
#include <Engine.Reflow/Widget/Text.hpp>
#include <Engine.Reflow/Widget/Button.hpp>
#include <Engine.Common/Make.hpp>

using namespace hg::engine::reflow;
using namespace hg::editor::ui;
using namespace hg;

[[nodiscard]] sptr<BoundStyleSheet> makeBreadStyle() {

    auto style { BoundStyleSheet::make() };

    style->pushStyle(
        {
            Style::BreadcrumbKey,
            nullptr,
            Style::get()->getStyleSheet(Style::BreadcrumbKey)
        }
    );

    return style;
}

[[nodiscard]] sptr<BoundStyleSheet> makeBreadEntryStyle() {

    auto style { BoundStyleSheet::make() };

    style->pushStyle(
        {
            Style::TextButtonKey,
            nullptr,
            Style::get()->getStyleSheet(Style::TextButtonKey)
        }
    );

    style->pushStyle(
        {
            Style::ButtonRaisedKey,
            style::isRaised,
            Style::get()->getStyleSheet(Style::ButtonRaisedKey)
        }
    );

    return style;
}

[[nodiscard]] sptr<BoundStyleSheet> makeBreadTitleStyle() {

    auto style { BoundStyleSheet::make() };

    style->pushStyle(
        {
            Style::TitleSmallKey,
            nullptr,
            Style::get()->getStyleSheet(Style::TitleSmallKey)
        }
    );

    style->pushStyle(
        {
            Style::ButtonRaisedKey,
            style::isNever,
            Style::get()->getStyleSheet(Style::TitleRaisedKey)
        }
    );

    return style;
}

[[nodiscard]] sptr<BoundStyleSheet> makeBreadSpaceStyle() {

    auto style {
        BoundStyleSheet::make(
            StyleSheet {
                .minHeight = { true, ReflowUnit { ReflowUnitType::eAbsolute, 16.F } }
            }
        )
    };

    style->pushStyle(
        {
            Style::TitleSmallKey,
            nullptr,
            Style::get()->getStyleSheet(Style::TitleSmallKey)
        }
    );

    return style;
}

Breadcrumb::Breadcrumb() :
    HBox(makeBreadStyle()) {}

Breadcrumb::~Breadcrumb() {
    clearNavEntries();
}

void Breadcrumb::addNavEntry(cref<AssocKey<string>> key_, cref<string> title_, cref<fs::Url> value_) {

    if (_STD ranges::find(
        _entries.begin(),
        _entries.end(),
        key_,
        [](cref<BreadcrumbEntry> entry_) {
            return entry_.key;
        }
    ) != _entries.end()) {
        return;
    }

    /**/

    if (not _entries.empty()) {

        auto spacer { make_sptr<Text>(makeBreadSpaceStyle()) };
        spacer->setText(R"(>)");

        this->addChild(spacer);
    }

    /**/

    auto button { make_sptr<Button>(makeBreadEntryStyle()) };
    auto title { make_sptr<Text>(makeBreadTitleStyle()) };

    title->setText(title_);
    button->addChild(title);

    /**/

    [[maybe_unused]] auto _ = button->addOnClick(
        [breadcrumb = weak_from_this(), url = value_](const auto&) {

            if (breadcrumb.expired()) {
                return;
            }

            _STD static_pointer_cast<Breadcrumb, Widget>(breadcrumb.lock())->handleAction(url);
        }
    );

    /**/

    this->addChild(button);

    /**/

    _entries.push_back({ key_, button });
}

void Breadcrumb::removeNavEntry(cref<AssocKey<string>> key_) {

    const auto where {
        _STD ranges::find(
            _entries.begin(),
            _entries.end(),
            key_,
            [](const auto& entry_) {
                return entry_.key;
            }
        )
    };

    if (where == _entries.end()) {
        return;
    }

    /**/

    const auto button { where->widget.lock() };
    const auto whereChild { _STD ranges::find(_children.begin(), _children.end(), button) };

    if (whereChild == _children.end()) {
        __debugbreak();
        return;
    }

    if (whereChild + 1 != _children.end() && _children.size() > 1) {
        const auto spacer { whereChild + 1 };
        this->removeChild(*spacer);
    }

    this->removeChild(button);
}

void Breadcrumb::clearNavEntries() {
    this->clearChildren();
    _entries.clear();
}

void Breadcrumb::handleAction(cref<fs::Url> url_) {
    for (const auto& pair : _actions) {
        pair.second(url_);
    }
}

u64 Breadcrumb::onAction(mref<action_fnc_type> fnc_) {
    const auto next { ++_areg };
    _actions.push_back(_STD make_pair(next, _STD move(fnc_)));
    return next;
}

void Breadcrumb::offAction(u64 action_) {

    const auto where = _STD ranges::remove(
        _actions.begin(),
        _actions.end(),
        action_,
        [](const auto& pair_) {
            return pair_.first;
        }
    );

    _actions.erase(where.begin(), where.end());
}

sptr<Breadcrumb> Breadcrumb::make() {
    return sptr<Breadcrumb>(new Breadcrumb());
}
