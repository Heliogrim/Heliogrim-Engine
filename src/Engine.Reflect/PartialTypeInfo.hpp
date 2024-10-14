#pragma once

#include <Engine.Common/Wrapper.hpp>

#include "FullTypeInfo.hpp"

namespace hg::refl {
	struct PartialTypeInfo final {
	public:
		using this_type = PartialTypeInfo;
		using meta_class_type = MetaClass;

	public:
		constexpr PartialTypeInfo() noexcept = default;

		constexpr PartialTypeInfo(mref<FullTypeInfo> fullTypeInfo_) noexcept :
			meta(fullTypeInfo_.meta) {}

		constexpr explicit PartialTypeInfo(const __restricted_ptr<const meta_class_type> meta_) noexcept :
			meta(meta_) {}

		constexpr PartialTypeInfo(mref<PartialTypeInfo>) noexcept = default;

		constexpr PartialTypeInfo(cref<PartialTypeInfo>) noexcept = default;

		constexpr ~PartialTypeInfo() noexcept = default;

	public:
		constexpr ref<PartialTypeInfo> operator=(mref<PartialTypeInfo>&& other_) noexcept = default;

		constexpr ref<PartialTypeInfo> operator=(cref<PartialTypeInfo> other_) = default;

	public:
		__restricted_ptr<const meta_class_type> meta;

		[[nodiscard]] constexpr bool exact(_In_ cref<PartialTypeInfo> check_) const noexcept {
			// Question: Do we need `meta->exact(check_.meta->typeId())`?
			return meta == check_.meta;
		}

		[[nodiscard]] constexpr bool is(_In_ cref<PartialTypeInfo> check_) const noexcept {
			return meta == check_.meta || (meta != nullptr && check_.meta != nullptr && meta->is(check_.meta->typeId()));
		}

		template <typename Type_>
		constexpr static PartialTypeInfo from() noexcept {
			return PartialTypeInfo { TypedMetaClass<Type_>::get() };
		}
	};
}
