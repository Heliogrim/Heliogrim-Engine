#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::cfg {
	template <typename Ty_>
	concept IsConfigValue = IsAnyOf<Ty_, std::monostate, bool, u32, f32, StringView, String>;

	class ConfigValue : public std::variant<std::monostate, bool, u32, f32, StringView, String> {
	public:
		using base_type = std::variant<std::monostate, bool, u32, f32, StringView, String>;
		using base_type::base_type;
	};
}
