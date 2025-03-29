#include "MenuItem.hpp"

#include <format>
#include <Engine.Reflow/Widget/Layer/Host.hpp>
#include <Engine.Reflow/Window/Window.hpp>

#include "Menu.hpp"

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

	::hg::assertrt(_runtimeHost == nullptr);

	auto window = std::static_pointer_cast<Window>(root());
	::hg::assertrt(window != nullptr);

	const auto position = getLayoutState().layoutOffset + math::fvec2 { getLayoutState().layoutSize.x, 0.F };

	_runtimeHost = make_sptr<Host>(
		_subMenu,
		position,
		math::fvec2 { std::numeric_limits<f32>::infinity(), std::numeric_limits<f32>::infinity() }
	);
	_subMenu->setParent(_runtimeHost);

	[[maybe_unused]] auto layer = window->requestLayer(_runtimeHost);
	_subMenu->markAsPending(true, true);
}

void MenuItem::hideSubMenu() {

	if (_runtimeHost == nullptr) {
		return;
	}

	auto window = std::static_pointer_cast<Window>(
		[this] {
			if (auto menuRoot = this->root()) {
				return menuRoot;
			}

			if (auto hostRoot = this->_runtimeHost->root()) {
				return hostRoot;
			}
			return SharedPtr<Widget> {};
		}()
	);
	::hg::assertrt(window != nullptr);

	window->dropLayer(_runtimeHost.get());
	_subMenu->setParent(nullptr);
	_runtimeHost.reset();
}

EventResponse MenuItem::invokeOnSelected(bool selected_) {

	if (selected_ && _subMenu != nullptr) {
		showSubMenu();
		return EventResponse::eHandled;
	}

	if (not selected_ && _runtimeHost != nullptr && _subMenu != nullptr) {
		hideSubMenu();
		return EventResponse::eHandled;
	}

	return EventResponse::eUnhandled;
}

/**/

SharedPtr<MenuItem> editor::ui::makeMenuItem() {
	return make_sptr<MenuItem>();
}
