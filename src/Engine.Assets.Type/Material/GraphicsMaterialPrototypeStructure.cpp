#include "GraphicsMaterialPrototype.hpp"

import Engine.Serialization;

using namespace hg::engine::serialization;
using namespace hg::engine::assets;
using namespace hg;

template <>
void access::Structure<GraphicsMaterialPrototype>::serialize(
	cref<GraphicsMaterialPrototype> self_,
	mref<StructScopedSlot> record_
) {

	// TODO:

	/**/

	Structure<Asset>::serialize(self_, ::hg::move(record_));
}

template <>
void access::Structure<GraphicsMaterialPrototype>::hydrate(
	cref<StructScopedSlot> record_,
	GraphicsMaterialPrototype& target_
) {

	Structure<Asset>::hydrate(record_, target_);

	/**/

	// TODO:
}