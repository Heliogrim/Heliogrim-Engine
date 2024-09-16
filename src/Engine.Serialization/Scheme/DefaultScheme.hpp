#pragma once

#include "Scheme.hpp"

namespace hg::engine::serialization {
	/**/

	struct NewDeleteAllocator {
		template <CompleteType TargetType_>
		[[nodiscard]] constexpr TargetType_* alloc(auto&&... args_) const
			requires std::is_constructible_v<TargetType_, decltype(args_)...> {
			return new TargetType_(std::forward<decltype(args_)>(args_)...);
		}
	};

	/**/

	template <CompleteType Type_>
	struct DefaultScheme : public Scheme<Type_> {
		template <typename>
		[[nodiscard]] static decltype(auto) instantiate(auto&& allocator_, auto&&... args_) {
			return std::forward<decltype(allocator_)>(allocator_).template alloc<Type_>(
				std::forward<decltype(args_)>(args_)...
			);
		}

		template <typename AccessType_>
		[[nodiscard]] static decltype(auto) deserialize(
			_In_ cref<RecordScopedSlot> slot_,
			auto&& allocator_,
			auto&&... args_
		) {
			auto* target = AccessType_::template instantiate(
				std::forward<decltype(allocator_)>(allocator_),
				std::forward<decltype(args_)>(args_)...
			);
			AccessType_::hydrate(slot_.asStruct(), *target);
			return target;
		}
	};
}
