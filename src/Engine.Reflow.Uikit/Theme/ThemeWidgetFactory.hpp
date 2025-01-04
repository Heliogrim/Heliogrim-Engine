#pragma once

#include "WidgetFactory.hpp"

namespace hg::engine::reflow::uikit {
	class ThemeWidgetFactory :
		public WidgetFactory {
	public:
		using this_type = ThemeWidgetFactory;

	public:
		~ThemeWidgetFactory() noexcept override = default;
	};
}
