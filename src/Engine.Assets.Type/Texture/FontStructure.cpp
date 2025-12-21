#include <Engine.Serialization/Access/Structure.hpp>
#include <Engine.Serialization/Structure/IntegralScopedSlot.hpp>
#include <Engine.Serialization/Structure/SliceScopedSlot.hpp>
#include <Engine.Serialization/Structure/StructScopedSlot.hpp>

#include "FontAsset.hpp"

using namespace hg::engine::assets;
using namespace hg::engine::serialization;
using namespace hg;

template <>
void access::Structure<FontAsset>::serialize(const FontAsset& self_, mref<StructScopedSlot> record_) {

	Vector<string> sources {};
	sources.reserve(self_._sources.size());

	for (const auto& entry : self_._sources) {
		sources.push_back(string { entry.encode() });
	}

	record_.insertSlot<string, Vector>("sources") << sources;

	/**/

	Structure<Asset>::serialize(self_, ::hg::move(record_));
}

template <>
void access::Structure<FontAsset>::hydrate(cref<StructScopedSlot> record_, FontAsset& target_) {

	Structure<Asset>::hydrate(record_, target_);

	/**/

	Vector<string> sources {};
	record_.getSlot<string, Vector>("sources") >> sources;

	target_._sources.reserve(sources.size());
	for (const auto& entry : sources) {

		if (entry.size() < 8uLL) {
			continue;
		}

		target_._sources.emplace_back("file"sv, entry.substr(7uLL));
	}

}