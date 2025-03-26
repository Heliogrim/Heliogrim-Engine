#include "Menu.hpp"

#include <format>
#include <Engine.Reflow.Uikit/Atom/Layout/VerticalLayout.hpp>
#include <Engine.Reflow.Uikit/Molecule/Layout/Stack.hpp>

#include "MenuItem.hpp"

using namespace hg::editor::ui;
using namespace hg::engine::reflow;
using namespace hg;

editor::ui::Menu::Menu(
	mref<SharedPtr<uikit::Stack>> content_,
	mref<SharedPtr<uikit::VerticalLayout>> layout_
) :
	CompoundWidget(::hg::move(content_), ReflowClassList {}, nullptr),
	_layout(::hg::move(layout_)) {}

editor::ui::Menu::~Menu() = default;

string editor::ui::Menu::getTag() const noexcept {
	return std::format(R"(Menu <{:#x}>)", reinterpret_cast<u64>(this));
}

void editor::ui::Menu::addMenuItem(mref<SharedPtr<MenuItem>> menuItem_) {
	_layout->addChild(::hg::move(menuItem_));
}

SharedPtr<editor::ui::Menu> editor::ui::makeMenu() {

	auto stack = make_sptr<uikit::Stack>();
	auto layout = make_sptr<uikit::VerticalLayout>();
	stack->addChild(clone(layout));

	auto menu = make_sptr<Menu>(clone(stack), hg::move(layout));
	stack->setParent(menu);

	return menu;
}
