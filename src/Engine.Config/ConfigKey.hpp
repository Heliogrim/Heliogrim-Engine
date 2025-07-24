#pragma once

#include <type_traits>
#include <Engine.Common/String.hpp>
#include <Engine.Reflect/Compile/String.hpp>

namespace hg::engine::cfg {
	template <typename Ty_>
	concept EnumOrString = std::is_enum_v<Ty_> ||
		std::is_same_v<Ty_, StringView> ||
		std::is_same_v<Ty_, CompileString>;

	template <typename Ty_>
	concept IsConfigKey = EnumOrString<Ty_>;

	using InternalConfigKey = decltype(std::declval<CompileString>().hash());
}
