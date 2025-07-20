#include "Breadcrumb.hpp"

#include <algorithm>
#include <ranges>
#include <Engine.Asserts/Breakpoint.hpp>
#include <Engine.Common/Make.hpp>
#include <Engine.Reflow/TextAlign.hpp>
#include <Engine.Reflow.Uikit/Atom/Text.hpp>
#include <Engine.Reflow.Uikit/Exp/Button.hpp>

using namespace hg::engine::reflow;
using namespace hg::editor::ui;
using namespace hg;

Breadcrumb::Breadcrumb() :
	HorizontalLayout() {
	/**/
	std::get<0>(getLayoutAttributes().attributeSets).update<attr::BoxLayout::minHeight>({ ReflowUnitType::eAbsolute, 20.F });
	std::get<0>(getLayoutAttributes().attributeSets).update<attr::BoxLayout::padding>(Padding { 4.F });

	std::get<1>(getLayoutAttributes().attributeSets).update<attr::FlexLayout::rowGap>(2.F);
	std::get<1>(getLayoutAttributes().attributeSets).update<attr::FlexLayout::align>(ReflowAlignment::eCenter);
}

Breadcrumb::~Breadcrumb() {
	clearNavEntries();
}

void Breadcrumb::addNavEntry(cref<AssocKey<string>> key_, cref<string> title_, cref<fs::Path> value_) {

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

		auto spacer { make_sptr<uikit::Text>() };
		spacer->getStyleAttributes().update<attr::TextStyle::textAlign>(TextAlign::eMiddleCenter);
		spacer->setText(R"(>)");

		this->addChild(spacer);
	}

	/**/

	auto title = make_sptr<uikit::Text>();
	title->setText(title_);
	auto button = uikit::makeButton(uikit::TextButtonCreateOptions { .level = 0, .text = ::hg::move(title) });

	/**/

	[[maybe_unused]] auto _ = button->onClick(
		[breadcrumb = weak_from_this(), url = value_](const auto&) {

			if (breadcrumb.expired()) {
				return EventResponse::eUnhandled;
			}

			std::static_pointer_cast<Breadcrumb, Widget>(breadcrumb.lock())->handleAction(url);
			return EventResponse::eHandled;
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
		::hg::breakpoint();
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

void Breadcrumb::handleAction(cref<fs::Path> path_) {
	for (const auto& pair : _actions) {
		pair.second(path_);
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
