#pragma once

namespace hg::editor::ui::service {
	class UiService {
	public:
		using this_type = UiService;

	protected:
		constexpr UiService() noexcept = default;

	public:
		constexpr virtual ~UiService() noexcept = default;
	};
}
