#pragma once

#include <type_traits>
#include <Engine.Common/Make.hpp>

#include "MapSlot.hpp"
#include "ScopedStructureSlot.hpp"
#include "__fwd.hpp"

namespace hg::engine::serialization {
	template <typename KeyType_, typename ValueType_, template <typename, typename...> typename MapType_>
	class MapScopedSlot final :
		TypeScopedSlot<MapType_<KeyType_, ValueType_>> {
	public:
		using this_type = MapScopedSlot<KeyType_, ValueType_, MapType_>;
		using underlying_type = TypeScopedSlot<MapType_<KeyType_, ValueType_>>;

		using value_type = typename underlying_type::value_type;

	public:
		MapScopedSlot(mref<ScopedSlotState> scopedState_, mref<StructureSlotState> state_) :
			underlying_type(
				std::move(scopedState_),
				make_sptr<MapSlot<KeyType_, ValueType_, MapType_>>(std::move(state_))
			) {}

		~MapScopedSlot() override = default;

	public:
		void operator<<(cref<value_type> value_) override {
			const ScopedSlotGuard guard { this, ScopedSlotGuardMode::eWrite };
			(*underlying_type::slot()) << value_;
		}

		void operator>>(ref<value_type> value_) const override {
			const ScopedSlotGuard guard { this, ScopedSlotGuardMode::eRead };
			(*underlying_type::slot()) >> value_;
		}
	};
}
