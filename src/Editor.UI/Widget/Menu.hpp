#pragma once

#include <Engine.Reflow/Widget/CompoundWidget.hpp>

namespace hg::engine::reflow::uikit {
	class VerticalLayout;
	class Stack;
}

namespace hg::editor::ui {
	class MenuItem;
	class MenuSpacer;
}

namespace hg::editor::ui {
	class Menu :
		public engine::reflow::CompoundWidget {
	public:
		using this_type = Menu;

	public:
		Menu(
			mref<SharedPtr<engine::reflow::uikit::Stack>> content_,
			mref<SharedPtr<engine::reflow::uikit::VerticalLayout>> layout_
		);

		~Menu() override;

	public:
		[[nodiscard]] string getTag() const noexcept override;

	private:
		Opt<ref<MenuItem>> _selected;
		SharedPtr<engine::reflow::uikit::VerticalLayout> _layout;

	public:
		void changeSelection(mref<Opt<ref<MenuItem>>> next_);

		void addMenuItem(mref<SharedPtr<MenuItem>> menuItem_);

		void addMenuSpacer(mref<SharedPtr<MenuSpacer>> menuSpacer_);
	};

	/**/

	[[nodiscard]] extern SharedPtr<Menu> makeMenu();
}
