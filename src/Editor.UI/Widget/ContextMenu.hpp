#pragma once

#include <Engine.Reflow/Widget/CompoundWidget.hpp>

#include "Menu.hpp"

namespace hg::editor::ui {
	class ContextMenu :
		public engine::reflow::CompoundWidget {
	public:
		using this_type = ContextMenu;

	public:
		ContextMenu(mref<SharedPtr<Menu>> menu_);

		~ContextMenu() override;

	public:
		[[nodiscard]] string getTag() const noexcept override;
	};
}
