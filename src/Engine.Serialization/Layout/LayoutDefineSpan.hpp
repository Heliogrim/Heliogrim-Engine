#pragma once

#include "LayoutDefine.hpp"
#include "LayoutDefineValueType.hpp"

namespace hg::engine::serialization::layout {
	class LayoutDefineSpan final :
		public LayoutDefine {
	public:
		using this_type = LayoutDefineSpan;
		using type_traits = LayoutDefineValueTrait<LayoutDefineValueType::eSpan>;

	public:
		LayoutDefineSpan(const u64 offset_, cref<sptr<DataLayoutBase>> layout_, const u64 count_);

	protected:
		static ref<resource::Archive> loadImpl(
			ref<resource::Archive> archive_,
			cref<LayoutDefine> define_,
			cref<std::span<u8, std::dynamic_extent>> dst_
		);

		static ref<resource::Archive> storeImpl(
			ref<resource::Archive> archive_,
			cref<LayoutDefine> define_,
			cref<std::span<u8, std::dynamic_extent>> src_
		);
	};
}
