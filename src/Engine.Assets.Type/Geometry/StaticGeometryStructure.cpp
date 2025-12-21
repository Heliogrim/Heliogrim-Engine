#include "StaticGeometry.hpp"

/**/

#include <filesystem>
#include <Engine.Serialization.Structures/Common/Guid.hpp>
#include <Engine.Serialization.Structures/Common/TypeId.hpp>
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
void access::Structure<StaticGeometry>::serialize(cref<StaticGeometry> self_, mref<StructScopedSlot> record_) {

	Vector<string> sources {};
	sources.reserve(self_._sources.size());

	for (const auto& entry : self_._sources) {
		sources.push_back(entry.encode());
	}

	record_.insertSlot<string, Vector>("sources") << sources;

	auto materialSeq = record_.insertSlot<void>("materials").intoSeq();
	for (const auto& material : self_._meshMaterials) {
		Structure<Guid>::serialize(material, materialSeq.addRecordSlot().intoStruct());
	}
	materialSeq.leaveSlot();

	record_.insertSlot<u64>("vertexCount") << self_._vertexCount;
	record_.insertSlot<u64>("indexCount") << self_._indexCount;

	record_.insertSlot<u64>("clusterCount") << self_._clusterCount;
	record_.insertSlot<u64>("clusterDepth") << self_._clusterDepth;

	/**/

	Structure<Asset>::serialize(self_, ::hg::move(record_));
}

template <>
void access::Structure<StaticGeometry>::hydrate(cref<StructScopedSlot> record_, StaticGeometry& target_) {

	Structure<Asset>::hydrate(record_, target_);

	/**/

	Vector<string> sources {};
	record_.getSlot<string, Vector>("sources") >> sources;

	target_._sources.reserve(sources.size());
	for (const auto& entry : sources) {

		if (entry.size() < 8uLL) {
			continue;
		}

		target_._sources.push_back(fs::Url { "file"sv, entry.substr(7uLL) });
	}

	if (record_.hasRecordSlot("materials")) {

		const auto materialSeq = record_.getSlot<void>("materials").asSeq();
		const auto materialSeqCount = materialSeq.getRecordCount();

		for (auto index = 0uL; index < materialSeqCount; ++index) {

			auto materialGuid = AssetGuid {};
			Structure<Guid>::hydrate(materialSeq.getRecordSlot(index).asStruct(), materialGuid);
			target_._meshMaterials.emplace_back(materialGuid);
		}
	}

	record_.getSlot<u64>("vertexCount") >> target_._vertexCount;
	record_.getSlot<u64>("indexCount") >> target_._indexCount;

	record_.getSlot<u64>("clusterCount") >> target_._clusterCount;
	record_.getSlot<u64>("clusterDepth") >> target_._clusterDepth;
}