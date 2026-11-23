#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Reflow.Uikit;
#endif

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
