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
	mref<StructScopedSlot> record_
) {

	// TODO:

	/**/

	Structure<Asset>::serialize(self_, ::hg::move(record_));
}

template <>
void access::Structure<GfxMaterialPrototype>::hydrate(
	cref<StructScopedSlot> record_,
	GfxMaterialPrototype& target_
) {

	Structure<Asset>::hydrate(record_, target_);

	/**/

	// TODO:
}