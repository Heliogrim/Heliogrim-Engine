#pragma once

#include <Engine.Common/Meta/Support.hpp>
#include <Engine.Common/Meta/Type.hpp>

#include "MetaClass.hpp"
#include "../Compile/Map.hpp"
#include "../Inherit/Compile.hpp"
#include "../Template/QueryTypeId.hpp"

namespace hg {
	class ClassMetaBase;

	template <typename, typename>
	struct typed_meta_holder;
}

namespace hg {
	template <CompleteType TargetType_, typename TypeList_ = reflect::__type_list_lookup<meta::peeled_t<TargetType_>>>
	class TypedMetaClass;

	template <CompleteType TargetType_, typename... InheritTypes_>
	class TypedMetaClass<TargetType_, reflect::__type_list<InheritTypes_...>> final :
		public MetaClass {
	public:
		template <typename, typename>
		friend struct ::hg::typed_meta_holder;

	public:
		using this_type = TypedMetaClass<TargetType_, reflect::__type_list<InheritTypes_...>>;
		using target_type = meta::peeled_t<TargetType_>;

	public:
		[[nodiscard]] constexpr static const __restricted_ptr<const this_type> get() noexcept;

	private:
		consteval TypedMetaClass() noexcept :
			MetaClass((typename ::hg::refl::query_type_id<target_type>::result {})()) {}

		constexpr ~TypedMetaClass() override = default;

	private:
		CompileMap<type_id, ::std::nullptr_t, sizeof...(InheritTypes_)> _inheritance = make_compile_map<type_id, ::std::nullptr_t>(
			std::make_pair<type_id, ::std::nullptr_t>(
				(typename ::hg::refl::query_type_id<InheritTypes_>::result {})(),
				nullptr
			)...
		);

	public:
		[[nodiscard]] constexpr bool inherits(const ptr<const MetaClass> type_) const noexcept override {
			return inherits(type_->typeId());
		}

		[[nodiscard]] constexpr bool inherits(const type_id typeId_) const noexcept override {
			return _inheritance.contains(typeId_);
		}
	};

	/**/

	template <typename TargetType_, typename TypeList_>
	struct typed_meta_holder {
		constexpr static TypedMetaClass<TargetType_, TypeList_> instance {};
	};

	template <CompleteType TargetType_, typename... InheritTypes_>
	constexpr const __restricted_ptr<const typename TypedMetaClass<
		TargetType_, reflect::__type_list<InheritTypes_...>
	>::this_type> TypedMetaClass<TargetType_, reflect::__type_list<InheritTypes_...>>::get() noexcept {
		return &typed_meta_holder<TargetType_, reflect::__type_list<InheritTypes_...>>::instance;
	}
}
