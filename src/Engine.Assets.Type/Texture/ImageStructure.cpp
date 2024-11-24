#include "Image.hpp"

#include <filesystem>
#include <Engine.Serialization/Access/Structure.hpp>
#include <Engine.Serialization/Structure/IntegralScopedSlot.hpp>
#include <Engine.Serialization/Structure/SliceScopedSlot.hpp>
#include <Engine.Serialization/Structure/StructScopedSlot.hpp>

using namespace hg::engine::assets;
using namespace hg::engine::serialization;
using namespace hg;

template <>
void access::Structure<Image>::serialize(const Image& self_, mref<StructScopedSlot> slot_) {

	Structure<Guid>::serialize(self_._guid, slot_.insertStructSlot("__guid__"));
	slot_.insertSlot<u64>("__type__") << self_._type.data;
	slot_.insertSlot<string>("name") << self_._assetName;

	Vector<string> sources {};
	sources.reserve(self_._sources.size());

	for (const auto& entry : self_._sources) {
		sources.push_back(string { entry.encode() });
	}

	slot_.insertSlot<string, Vector>("sources") << sources;
}

template <>
void access::Structure<Image>::hydrate(cref<StructScopedSlot> slot_, Image& target_) {

	Structure<Guid>::hydrate(slot_.getSlot<void>("__guid__").asStruct(), target_._guid);
	slot_.getSlot<u64>("__type__") >> target_._type.data;
	slot_.getSlot<string>("name") >> target_._assetName;

	Vector<string> sources {};
	slot_.getSlot<string, Vector>("sources") >> sources;

	target_._sources.reserve(sources.size());
	for (const auto& entry : sources) {

		if (entry.size() < 8uLL) {
			continue;
		}

		target_._sources.push_back(fs::Url { "file"sv, entry.substr(7uLL) });
	}

}
