#include <Engine.Serialization/Access/Structure.hpp>
#include <Engine.Serialization/Structure/FloatScopedSlot.hpp>
#include <Engine.Serialization/Structure/IntegralScopedSlot.hpp>
#include <Engine.Serialization/Structure/StructScopedSlot.hpp>
#include <Engine.Serialization.Structures/Symbols.hpp>

#include "StaticGeometryComponent.hpp"

using namespace hg::engine::serialization;
using namespace hg;

template <>
void access::Structure<StaticGeometryComponent>::serialize(
	const StaticGeometryComponent& self_,
	mref<StructScopedSlot> slot_
) {

	slot_.insertSlot<u64>(Symbols::Type) << StaticGeometryComponent::typeId.data;

	/**/

}

template <>
void access::Structure<StaticGeometryComponent>::hydrate(
	cref<StructScopedSlot> slot_,
	StaticGeometryComponent& target_
) {

	component_type_id checkTypeId {};
	slot_.getSlot<u64>(Symbols::Type) >> checkTypeId.data;
	::hg::assertrt(checkTypeId == StaticGeometryComponent::typeId);

	/**/
}
