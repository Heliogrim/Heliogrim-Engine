#pragma once

#include <Engine.Reflow.Uikit/Atom/Layout/BoxLayout.hpp>

namespace hg::editor::ui {
	class MenuSpacer :
		public engine::reflow::uikit::BoxLayout {
	public:
		MenuSpacer();

		~MenuSpacer() override;

	public:
		[[nodiscard]] String getTag() const noexcept override;
	};

	/**/

	[[nodiscard]] extern SharedPtr<MenuSpacer> makeMenuSpacer();
}
