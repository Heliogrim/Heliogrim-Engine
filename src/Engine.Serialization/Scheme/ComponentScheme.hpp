#pragma once

#include <memory>
#include <utility>
#include <Engine.ACS/ComponentGuid.hpp>
#include <Engine.ACS/Registry.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.Serialization/Structure/RecordScopedSlot.hpp>
#include <Engine.Serialization/Structure/StructScopedSlot.hpp>
#include <Heliogrim/Component/CachedActorPointer.hpp>

#include "Scheme.hpp"
#include "__fwd.hpp"

namespace hg::engine::serialization {
	/**/

	template <CompleteType Type_> requires std::derived_from<Type_, LogicComponent>
	struct ComponentScheme : public Scheme<Type_> {
		template <typename>
		[[nodiscard]] static decltype(auto) instantiate(
			nmpt<acs::Registry> allocator_,
			_In_ ref<const ComponentGuid> componentGuid_,
			_Inout_ ref<Actor> owner_,
			_Maybenull_ nmpt<HierarchyComponent> parent_,
			auto&&... args_
		) {
			return allocator_->acquireActorComponent<Type_, CachedActorPointer, ptr<HierarchyComponent>>(
				componentGuid_,
				CachedActorPointer { owner_.guid(), std::addressof(owner_) },
				parent_.get(),
				std::forward<decltype(args_)>(args_)...
			);
		}

		template <typename AccessType_>
		[[nodiscard]] static decltype(auto) deserialize(
			_In_ cref<RecordScopedSlot> slot_,
			nmpt<acs::Registry> allocator_,
			_Inout_ ref<Actor> owner_,
			_Maybenull_ nmpt<HierarchyComponent> parent_,
			auto&&... args_
		) {
			const auto componentGuid = get_guid_slot<ComponentGuid>(slot_.asStruct());

			auto target = AccessType_::template instantiate(
				std::forward<decltype(allocator_)>(allocator_),
				componentGuid,
				std::forward<decltype(owner_)>(owner_),
				std::forward<decltype(parent_)>(parent_),
				std::forward<decltype(args_)>(args_)...
			);
			AccessType_::hydrate(slot_.asStruct(), *target);
			return target;
		}
	};
}
