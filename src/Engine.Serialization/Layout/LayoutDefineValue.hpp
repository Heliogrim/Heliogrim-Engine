#pragma once

#include "LayoutDefine.hpp"
#include "LayoutDefineValueType.hpp"

namespace hg::engine::serialization::layout {
	template <LayoutDefineValueType ValueType_>
	class LayoutDefineValue final :
		public LayoutDefine {
	public:
		using this_type = LayoutDefineValue<ValueType_>;
		using type_traits = LayoutDefineValueTrait<ValueType_>;

	public:
		constexpr explicit LayoutDefineValue(const u64 offset_) :
			LayoutDefine(offset_, type_traits::size) {

			static_assert(type_traits::is_inline && type_traits::is_constant_size && not type_traits::has_subtype);

			_load = this_type::loadImpl;
			_store = this_type::storeImpl;
		}

	protected:
		static ref<resource::Archive> loadImpl(
			ref<resource::Archive> archive_,
			cref<LayoutDefine> define_,
			cref<std::span<u8, std::dynamic_extent>> dst_
		) {

			using value_type = typename type_traits::type;
			using value_ptr_type = ptr<value_type>;

			return archive_ >> (*reinterpret_cast<value_ptr_type>(&dst_[define_._offset]));
		}

		static ref<resource::Archive> storeImpl(
			ref<resource::Archive> archive_,
			cref<LayoutDefine> define_,
			cref<std::span<u8, std::dynamic_extent>> src_
		) {

			using value_type = typename type_traits::type;
			using value_ptr_type = const ptr<value_type>;

			return archive_ << (*reinterpret_cast<value_ptr_type>(&src_[define_._offset]));
		}
	};
}
