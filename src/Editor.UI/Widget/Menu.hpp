#pragma once

#include <Engine.Reflow/Widget/CompoundWidget.hpp>

namespace hg::editor::ui {
	class Menu :
		public engine::reflow::CompoundWidget {
	public:
		using this_type = Menu;

	public:
		Menu();

		~Menu() override;

	public:
		[[nodiscard]] string getTag() const noexcept override;
	};
}
