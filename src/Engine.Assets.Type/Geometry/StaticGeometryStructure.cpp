#include "StaticGeometry.hpp"

#include <filesystem>
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
void access::Structure<StaticGeometry>::serialize(cref<StaticGeometry> self_, mref<StructScopedSlot> slot_) {

	Structure<Guid>::serialize(self_._guid, slot_.insertSlot<void>("__guid__").intoStruct());
	slot_.insertSlot<u64>("__type__") << self_._type.data;
	slot_.insertSlot<string>("name") << self_._assetName;

	/**/

	Vector<string> sources {};
	sources.reserve(self_._sources.size());

	for (const auto& entry : self_._sources) {
		sources.push_back(entry.encode());
	}

	slot_.insertSlot<string, Vector>("sources") << sources;

	slot_.insertSlot<u64>("vertexCount") << self_._vertexCount;
	slot_.insertSlot<u64>("indexCount") << self_._indexCount;

	slot_.insertSlot<u64>("clusterCount") << self_._clusterCount;
	slot_.insertSlot<u64>("clusterDepth") << self_._clusterDepth;
}

template <>
void access::Structure<StaticGeometry>::hydrate(cref<StructScopedSlot> slot_, StaticGeometry& target_) {

	Structure<Guid>::hydrate(slot_.getRecordSlot("__guid__").asStruct(), target_._guid);
	slot_.getSlot<u64>("__type__") >> target_._type.data;
	slot_.getSlot<string>("name") >> target_._assetName;

	/**/

	Vector<string> sources {};
	slot_.getSlot<string, Vector>("sources") >> sources;

	target_._sources.reserve(sources.size());
	for (const auto& entry : sources) {

		if (entry.size() < 8uLL) {
			continue;
		}

		target_._sources.push_back(fs::Url { "file"sv, entry.substr(7uLL) });
	}

	slot_.getSlot<u64>("vertexCount") >> target_._vertexCount;
	slot_.getSlot<u64>("indexCount") >> target_._indexCount;

	slot_.getSlot<u64>("clusterCount") >> target_._clusterCount;
	slot_.getSlot<u64>("clusterDepth") >> target_._clusterDepth;
}
