#pragma once

#include <Engine.Common/Wrapper.hpp>

#include "Inherit/Concept.hpp"
#include "Meta/MetaClass.hpp"

namespace hg {
	template <typename Type_>
	struct IsTypeFn;

	template <typename Type_> requires (ClassHasMeta<Type_>)
	struct IsTypeFn<Type_> {
		template <typename Arg_> requires ClassHasMeta<std::remove_cvref_t<Arg_>>
		[[nodiscard]] constexpr bool operator()(Arg_&& arg_) const noexcept {
			return std::forward<Arg_>(arg_).getMetaClass()->template is<Type_>();
		}
	};

	template <typename Type_> requires (not ClassHasMeta<Type_>)
	struct IsTypeFn<Type_> {
		static_assert(ClassHasMeta<Type_>);
	};

	template <CompleteType Type_>
	static constexpr IsTypeFn<Type_> IsType {};
}
