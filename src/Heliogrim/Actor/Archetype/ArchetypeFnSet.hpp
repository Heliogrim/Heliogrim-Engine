#pragma once

#include <Engine.Serialization/FnSets.hpp>
#include <Engine.Serialization/Access/Structure.hpp>
#include <Engine.Serialization/Structure/RecordScopedSlot.hpp>
#include <Engine.Serialization/Structure/StructScopedSlot.hpp>

#include "ActorClass.hpp"
#include "../Actor.hpp"

namespace hg {
	/* Level scheme of an archetype; the class is captured here because Engine.Level cannot resolve actor classes. */
	// `class_` must be a static ActorClass (`X::actorClass()`); the scheme keeps the pointer.
	[[nodiscard]] inline engine::serialization::ActorFnSet MakeArchetypeFnSet(const ptr<const ActorClass> class_) {
		return engine::serialization::ActorFnSet {
			.serialize = [](ref<const Actor> actor_, mref<engine::serialization::StructScopedSlot> slot_) -> void {
				engine::serialization::access::Structure<Actor>::serialize(actor_, ::hg::move(slot_));
			},
			.deserialize = [class_](
				ref<const engine::serialization::RecordScopedSlot> slot_,
				nmpt<engine::acs::Registry>
			) -> ptr<Actor> {
				const auto record = slot_.asStruct();
				return CreateActor(class_, SerializedActor { record }).get().release();
			}
		};
	}
}
