#pragma once

#include "Engine.Reflow/Widget/CompoundWidget.hpp"

namespace hg::engine::reflow {
	class Host;
}

namespace hg::editor::ui {
	class Menu;
}

namespace hg::editor::ui {
	class MenuItem :
		public engine::reflow::CompoundWidget {
	public:
		using this_type = MenuItem;

	public:
		MenuItem();

		~MenuItem() override;

	public:
		[[nodiscard]] String getTag() const noexcept override;

	private:
		SharedPtr<Menu> _subMenu;
		SharedPtr<engine::reflow::Host> _runtimeHost;

	public:
		void setContent(mref<SharedPtr<Widget>> content_);

		void setSubMenu(mref<SharedPtr<Menu>> subMenu_);

	private:
		void showSubMenu();

		void hideSubMenu();

	public:
		engine::reflow::EventResponse invokeOnFocus(cref<engine::reflow::FocusEvent> event_) override;

		engine::reflow::EventResponse invokeOnBlur(cref<engine::reflow::FocusEvent> event_) override;
	};

	/**/

	[[nodiscard]] extern SharedPtr<MenuItem> makeMenuItem();
}
