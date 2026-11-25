#include "MenuItem.hpp"

#include "Menu.hpp"

import Engine.Reflow;
import std;

using namespace hg::editor::ui;
using namespace hg::engine::reflow;
using namespace hg;

MenuItem::MenuItem() :
	CompoundWidget(ReflowClassList {}, nullptr),
	_subMenu() {}

MenuItem::~MenuItem() {
	hideSubMenu();
};

String MenuItem::getTag() const noexcept {
	return std::format(R"(MenuItem <{:#x}>)", reinterpret_cast<u64>(this));
}

void MenuItem::setContent(mref<SharedPtr<Widget>> content_) {
	content_->setParent(shared_from_this());
	auto prev = _children.setChild(::hg::move(content_));
	prev->setParent(nullptr);

	markAsPending(true, true);
}

void MenuItem::setSubMenu(mref<SharedPtr<Menu>> subMenu_) {
	_subMenu = ::hg::move(subMenu_);
}

void MenuItem::showSubMenu() {

	::hg::assertrt(not _subMenuPopup.has_value());

	auto window = std::static_pointer_cast<Window>(root());
	::hg::assertrt(window != nullptr);

	const auto& layout = getLayoutState();
	_subMenuPopup = WindowManager::get()->openPopup(
		window,
		_subMenu,
		PopupAnchor { .offset = layout.layoutOffset, .size = layout.layoutSize, .side = PopupSide::eRight, .align = PopupAlign::eStart },
		weak_from_this(),
		None,
		[this, self = weak_from_this()]() {
			if (not self.expired()) {
				_subMenuPopup = None;
			}
		}
	);
}

void MenuItem::hideSubMenu() {

	// The manager is gone at shutdown, its router drops the popups itself.
	if (const auto& manager = WindowManager::get(); _subMenuPopup.has_value() && manager != nullptr) {
		manager->closePopup(*_subMenuPopup);
	}
}

EventResponse MenuItem::invokeOnSelected(bool selected_) {

	if (selected_ && _subMenu != nullptr) {
		showSubMenu();
		return EventResponse::eHandled;
	}

	if (not selected_ && _subMenuPopup.has_value() && _subMenu != nullptr) {
		hideSubMenu();
		return EventResponse::eHandled;
	}

	return EventResponse::eUnhandled;
}

/**/

SharedPtr<MenuItem> editor::ui::makeMenuItem() {
	return make_sptr<MenuItem>();
}
