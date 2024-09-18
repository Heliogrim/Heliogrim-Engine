#pragma once

#include "StructureSlotType.hpp"
#include "StructureSlotTypeTraits.hpp"

namespace hg::engine::serialization {
	struct StructureSlotHeader {
	public:
		StructureSlotType type = StructureSlotType::eUndefined;
		s64 size = -1;
		//
		bool sizeInferred = false;

	public:
		template <StructureSlotType SlotType_>
		[[nodiscard]] constexpr static StructureSlotHeader from() noexcept {
			return {
				SlotType_,
				StructureSlotTypeTraits<SlotType_>::const_size,
				StructureSlotTypeTraits<SlotType_>::const_size > 0
			};
		}

		[[nodiscard]] static StructureSlotHeader from(const StructureSlotType type_) noexcept {
			return {
				type_,
				StructureSlotTypeTrait::const_size(type_),
				StructureSlotTypeTrait::const_size(type_) > 0
			};
		}
	};
}
