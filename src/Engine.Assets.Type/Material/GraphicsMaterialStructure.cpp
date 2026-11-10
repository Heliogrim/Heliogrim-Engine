#include "GraphicsMaterial.hpp"

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
void access::Structure<GraphicsMaterial>::serialize(cref<GraphicsMaterial> self_, mref<StructScopedSlot> record_) {

	// TODO:

	/**/

	Structure<Asset>::serialize(self_, ::hg::move(record_));
}

template <>
void access::Structure<GraphicsMaterial>::hydrate(cref<StructScopedSlot> record_, GraphicsMaterial& target_) {

	Structure<Asset>::hydrate(record_, target_);

	/**/

	// TODO:
}