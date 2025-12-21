#include "Asset.hpp"

#include <Engine.Serialization/Access/Structure.hpp>
#include <Engine.Serialization/Structure/IntegralScopedSlot.hpp>
#include <Engine.Serialization/Structure/StructScopedSlot.hpp>
#include <Engine.Serialization.Structures/Symbols.hpp>
#include <Engine.Serialization.Structures/Common/Guid.hpp>
#include <Engine.Serialization.Structures/Common/TypeId.hpp>

using namespace hg::engine::assets;
using namespace hg::engine::serialization;
using namespace hg;

template <>
void access::Structure<Asset>::serialize(const Asset& self_, mref<StructScopedSlot> record_) {

	insert_guid_slot(record_, self_._guid);
	insert_typeId_slot(record_, self_._type);

	record_.insertSlot<string>("name") << self_._assetName;
	record_.insertSlot<string>("vfs") << String { std::from_range, self_._assetVfsUrl.getAssetPath().asByteSpan() };
}

template <>
void access::Structure<Asset>::hydrate(cref<StructScopedSlot> record_, Asset& target_) {

	// TODO: Make a convenient way to define data migration paths when loading data
	if (has_guid_slot(record_)) {
		target_._guid = get_guid_slot<AssetGuid>(record_);
	} else {
		Structure<Guid>::hydrate(record_.getSlot<void>("__guid__"sv).asStruct(), target_._guid);
	}

	if (has_typeId_slot(record_)) {
		target_._type = get_typeId_slot<AssetTypeId>(record_);
	} else {
		record_.getSlot<u64>("__type__") >> target_._type.data;
	}

	record_.getSlot<string>("name") >> target_._assetName;

	if (record_.hasRecordSlot("vfs")) {
		auto vfsUrlPath = String {};
		record_.getSlot<string>("vfs") >> vfsUrlPath;
		target_.setAssetVfsUrl(AssetUrl { ::hg::move(vfsUrlPath), target_._assetName });
	}
}
