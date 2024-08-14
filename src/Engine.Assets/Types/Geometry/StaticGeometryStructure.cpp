#include "StaticGeometry.hpp"

#include <Engine.Serialization/Access/Structure.hpp>
#include <Engine.Serialization/Structure/IntegralScopedSlot.hpp>
#include <Engine.Serialization/Structure/SliceScopedSlot.hpp>
#include <Engine.Serialization/Structure/StringScopedSlot.hpp>
#include <Engine.Serialization/Structure/StructScopedSlot.hpp>
#include <Engine.Serialization/Structure/SeqScopedSlot.hpp>

using namespace hg::engine::serialization;
using namespace hg::engine::assets;
using namespace hg;

template <>
void access::Structure<StaticGeometry>::serialize(const StaticGeometry* const self_, mref<RecordScopedSlot> slot_) {

	auto root = slot_.intoStruct();

	Structure<Guid>::serialize(&self_->_guid, root.insertSlot<void>("__guid__"));
	root.insertSlot<u64>("__type__") << self_->_type.data;
	root.insertSlot<string>("name") << self_->_assetName;

	/**/

	Vector<string> sources {};
	sources.reserve(self_->_sources.size());

	for (const auto& entry : self_->_sources) {
		sources.push_back(entry.encode());
	}

	root.insertSlot<string, Vector>("sources") << sources;

	root.insertSlot<u64>("vertexCount") << self_->_vertexCount;
	root.insertSlot<u64>("indexCount") << self_->_indexCount;

	root.insertSlot<u64>("clusterCount") << self_->_clusterCount;
	root.insertSlot<u64>("clusterDepth") << self_->_clusterDepth;
}

template <>
void access::Structure<StaticGeometry>::deserialize(StaticGeometry* const self_, mref<RecordScopedSlot> slot_) {

	const auto root = slot_.intoStruct();

	Structure<Guid>::deserialize(&self_->_guid, root.getSlot<void>("__guid__"));
	root.getSlot<u64>("__type__") >> self_->_type.data;
	root.getSlot<string>("name") >> self_->_assetName;

	/**/

	Vector<string> sources {};
	root.getSlot<string, Vector>("sources") >> sources;

	self_->_sources.reserve(sources.size());
	for (const auto& entry : sources) {

		if (entry.size() < 8uLL) {
			continue;
		}

		self_->_sources.push_back(fs::Url { "file"sv, entry.substr(7uLL) });
	}

	root.getSlot<u64>("vertexCount") >> self_->_vertexCount;
	root.getSlot<u64>("indexCount") >> self_->_indexCount;

	root.getSlot<u64>("clusterCount") >> self_->_clusterCount;
	root.getSlot<u64>("clusterDepth") >> self_->_clusterDepth;
}
