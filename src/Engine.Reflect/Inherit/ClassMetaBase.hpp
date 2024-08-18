#pragma once
#include <Engine.Common/Wrapper.hpp>

#include "Compile.hpp"

namespace hg {
	template <typename>
	class InheritBase;

	template <typename, typename, typename...>
	class InheritMeta;

	template <typename>
	struct lookup;
}

namespace hg {
	class macro_novtable ClassMetaBase {
	public:
		template <typename>
		friend class InheritBase;

		template <typename, typename, typename...>
		friend class InheritMeta;

		template <typename>
		friend struct lookup;

		constexpr static const char* name = "ClassMetaBase";

	public:
		using this_type = ClassMetaBase;

		using __inherit_types = reflect::__type_list<>;

	public:
		constexpr ClassMetaBase() noexcept = default;

		constexpr ClassMetaBase(cref<this_type>) noexcept = default;

		constexpr ClassMetaBase(mref<this_type>) noexcept = default;

		constexpr virtual ~ClassMetaBase() noexcept = default;

	public:
		constexpr ref<this_type> operator=(cref<this_type>) noexcept = default;

		constexpr ref<this_type> operator=(mref<this_type>) noexcept = default;

	private:
		__restricted_ptr<const class MetaClass> _meta = nullptr;

	public:
		[[nodiscard]] std::add_const_t<__restricted_ptr<const class MetaClass>> getMetaClass() const noexcept {
			return _meta;
		}
	};
}
