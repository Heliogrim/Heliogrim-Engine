#include "Menu.hpp"

#include <format>
#include <Engine.Reflow.Uikit/Atom/Paint.hpp>
#include <Engine.Reflow.Uikit/Atom/Layout/VerticalLayout.hpp>
#include <Engine.Reflow.Uikit/Molecule/Layout/Stack.hpp>

#include "MenuItem.hpp"
#include "MenuSpacer.hpp"

using namespace hg::editor::ui;
using namespace hg::engine::reflow;
using namespace hg;

editor::ui::Menu::Menu(
	mref<SharedPtr<uikit::Stack>> content_,
	mref<SharedPtr<uikit::VerticalLayout>> layout_
) :
	CompoundWidget(::hg::move(content_), ReflowClassList {}, nullptr),
	_selected(None),
	_layout(::hg::move(layout_)) {}

editor::ui::Menu::~Menu() = default;

string editor::ui::Menu::getTag() const noexcept {
	return std::format(R"(Menu <{:#x}>)", reinterpret_cast<u64>(this));
}

void editor::ui::Menu::changeSelection(mref<Opt<ref<MenuItem>>> next_) {

	if (_selected != None) {
		_selected->invokeOnSelected(false);
	}

	_selected = ::hg::move(next_);

	if (_selected != None) {
		_selected->invokeOnSelected(true);
	}
}

void editor::ui::Menu::addMenuItem(mref<SharedPtr<MenuItem>> menuItem_) {

	menuItem_->onMouseEnter(
		[this, item = menuItem_.get()](const auto& event_) {
			changeSelection(Some(*item));
			return EventResponse::eHandled;
		}
	);

	/**/

	_layout->addChild(::hg::move(menuItem_));
}

void editor::ui::Menu::addMenuSpacer(mref<SharedPtr<MenuSpacer>> menuSpacer_) {
	_layout->addChild(::hg::move(menuSpacer_));
}

SharedPtr<editor::ui::Menu> editor::ui::makeMenu() {

	auto stack = make_sptr<uikit::Stack>();

	auto paint = make_sptr<uikit::Paint>(ReflowClassList { "[Menu] > background"sv }, nullptr);
	stack->addChild(::hg::move(paint));

	auto layout = make_sptr<uikit::VerticalLayout>();
	std::get<0>(layout->getLayoutAttributes().attributeSets).update<attr::BoxLayout::padding>(Padding { 4.F });
	stack->addChild(clone(layout));

	auto menu = make_sptr<Menu>(clone(stack), hg::move(layout));
	stack->setParent(menu);

	return menu;
}
