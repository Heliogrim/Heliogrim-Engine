#pragma once

#include <Engine.ACS/Registry.hpp>
#include <Engine.ACS/Component/CachedActorPointer.hpp>
#include <Engine.ACS/Component/ComponentGuid.hpp>
#include <Engine.Serialization/Scheme/Scheme.hpp>
#include <Engine.Serialization/Structure/RecordScopedSlot.hpp>
#include <Engine.Serialization/Structure/StructScopedSlot.hpp>

#include "SelectScheme.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

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
