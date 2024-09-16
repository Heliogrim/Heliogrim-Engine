#include "GfxMaterialPrototype.hpp"

#include <Engine.Serialization/Access/Structure.hpp>
#include <Engine.Serialization/Structure/IntegralScopedSlot.hpp>
#include <Engine.Serialization/Structure/SeqScopedSlot.hpp>
#include <Engine.Serialization/Structure/SliceScopedSlot.hpp>
#include <Engine.Serialization/Structure/StringScopedSlot.hpp>
#include <Engine.Serialization/Structure/StructScopedSlot.hpp>

using namespace hg::engine::serialization;
using namespace hg::engine::assets;
using namespace hg;

template <>
void access::Structure<GfxMaterialPrototype>::serialize(
	cref<GfxMaterialPrototype> self_,
	mref<StructScopedSlot> slot_
) {

	Structure<Guid>::serialize(self_._guid, slot_.insertStructSlot("__guid__"));
	slot_.insertSlot<u64>("__type__") << self_._type.data;
	slot_.insertSlot<string>("name") << self_._assetName;

	/**/

	// TODO:
}

template <>
void access::Structure<GfxMaterialPrototype>::hydrate(
	cref<StructScopedSlot> slot_,
	GfxMaterialPrototype& target_
) {

	Structure<Guid>::hydrate(slot_.getStructSlot("__guid__"), target_._guid);
	slot_.getSlot<u64>("__type__") >> target_._type.data;
	slot_.getSlot<string>("name") >> target_._assetName;

	/**/

	// TODO:
}
