#include "Breadcrumb.hpp"

#include <Engine.Reflow/Widget/Text.hpp>
#include <Engine.Reflow/Widget/Button.hpp>
#include <Engine.Common/Make.hpp>

#include "Editor.UI/Theme/Theme.hpp"

using namespace hg::engine::reflow;
using namespace hg::editor::ui;
using namespace hg;

Breadcrumb::Breadcrumb() :
    HorizontalPanel() {
    /**/
    attr.maxWidth.setValue({ ReflowUnitType::eRelative, 1.F });
    attr.minHeight.setValue({ ReflowUnitType::eAbsolute, 20.F });
    attr.maxHeight.setValue({ ReflowUnitType::eRelative, 1.F });
    attr.rowGap.setValue(2.F);
    attr.padding.setValue(Padding { 4.F, 2.F });
    attr.align.setValue(ReflowAlignment::eCenter);
}

Breadcrumb::~Breadcrumb() {
    clearNavEntries();
}

void Breadcrumb::addNavEntry(cref<AssocKey<string>> key_, cref<string> title_, cref<fs::Url> value_) {

    const auto* const theme = Theme::get();

    if (std::ranges::find(
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

        auto spacer { make_sptr<Text>() };
        theme->applyLabel(spacer);
        spacer->attr.textAlign.setValue(TextAlign::eCenterMiddle);
        spacer->setText(R"(>)");

        this->addChild(spacer);
    }

    /**/

    auto button { make_sptr<Button>() };
    theme->applyTextButton(button);
    auto title { make_sptr<Text>() };
    theme->applyLabel(title);

    title->setText(title_);
    button->setChild(title);

    /**/

    [[maybe_unused]] auto _ = button->addOnClick(
        [breadcrumb = weak_from_this(), url = value_](const auto&) {

            if (breadcrumb.expired()) {
                return;
            }

            std::static_pointer_cast<Breadcrumb, Widget>(breadcrumb.lock())->handleAction(url);
        }
    );

    /**/

    this->addChild(button);

    /**/

    _entries.push_back({ key_, button });
}

void Breadcrumb::removeNavEntry(cref<AssocKey<string>> key_) {

    const auto where {
        std::ranges::find(
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
    const auto whereChild { std::ranges::find(_children.begin(), _children.end(), button) };

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
    _actions.push_back(std::make_pair(next, std::move(fnc_)));
    return next;
}

void Breadcrumb::offAction(u64 action_) {

    const auto where = std::ranges::remove(
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
