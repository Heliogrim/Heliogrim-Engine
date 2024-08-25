#pragma once

#include <concepts>
#include <Engine.Common/Wrapper.hpp>

#include "../Meta/TypedMetaClass.hpp"

namespace hg {
	class ClassMetaBase;
}

namespace hg {
	namespace {
		template <typename Type_>
		struct remove_restricted {
			using type = Type_;
		};

		template <typename Type_>
		struct remove_restricted<__restricted_ptr<Type_>> {
			using type = ptr<Type_>;
		};

		template <typename Type_>
		using remove_restricted_t = typename remove_restricted<Type_>::type;

		template <typename Left_, typename Right_>
		concept unconstrained_same = std::same_as<remove_restricted_t<Left_>, Right_>;

		template <class Type_>
		concept __DirectMetaInherit = requires(const Type_& obj_) {
			{ obj_.getMetaClass() } -> unconstrained_same<ptr<const TypedMetaClass<Type_>>>;
		};

		template <
			class Type_,
			class RetTy_ = std::invoke_result_t<decltype(&Type_::getMetaClass)(Type_)>>
		concept __IndirectMetaInhert = std::derived_from<Type_, typename RetTy_::target_type>;
	}

	template <class Type_>
	concept ClassHasMeta =
		(std::same_as<ClassMetaBase, Type_> || __DirectMetaInherit<Type_> || __IndirectMetaInhert<Type_>) &&
		__HasInheritTypeList<Type_> &&
		std::derived_from<Type_, ClassMetaBase>;
}
