#include "pch.h"

/**/

#include <Engine.ACS/Archetype/ArchetypeLayout.inl.hpp>
import Engine.Common;
import Engine.Graphics.Components;
import Engine.Resource.Archive;
import Engine.Serialization;
import Engine.Serialization.Structures;

using namespace hg::engine::serialization;
using namespace hg::engine::resource;
using namespace hg;

namespace {
	template <typename Type_>
	[[nodiscard]] uptr<Type_> make() {
		return make_uptr<Type_>(ComponentGuid {}, CachedActorPointer {}, nullptr);
	}

	template <typename Type_>
	void structuredRoundTrip(ref<const Type_> source_, ref<Type_> target_) {
		auto archive = BufferArchive {}; {
			auto structured = StructuredArchive { archive };
			access::Structure<Type_>::serialize(source_, structured.insertRootSlot().intoStruct());
		}
		archive.seek(0);
		auto structured = StructuredArchive { archive };
		access::Structure<Type_>::hydrate(structured.getRootSlot().asStruct(), target_);
	}

	template <typename Type_>
	void layoutRoundTrip(ref<const Type_> source_, ref<Type_> target_) {
		Vector<std::byte> bytes {};
		::hg::engine::acs::dump_payload(source_, bytes);
		ASSERT_EQ(bytes.size(), ::hg::engine::acs::payload_size<Type_>());
		ASSERT_EQ(::hg::engine::acs::check_payload<Type_>(bytes, 0uLL), ~0uLL);
		::hg::engine::acs::commit_payload(bytes, 0uLL, target_);
		LoadContext ctx { bytes.size() };
		::hg::engine::acs::resolve_payload(bytes, 0uLL, target_, ctx);
	}
}

// Without an asset database only unbound geometry and materials can round trip.
TEST(StaticGeometrySerialization, LayoutRoundTrip) {
	const auto source = make<StaticGeometryComponent>();
	const auto target = make<StaticGeometryComponent>();
	layoutRoundTrip(*source, *target);
	EXPECT_FALSE(target->getStaticGeometry().isValid());
	EXPECT_TRUE(target->getInstanceMaterials().empty());
}

TEST(StaticGeometrySerialization, StructuredRoundTrip) {
	const auto source = make<StaticGeometryComponent>();
	const auto target = make<StaticGeometryComponent>();
	structuredRoundTrip(*source, *target);
	EXPECT_FALSE(target->getStaticGeometry().isValid());
	EXPECT_TRUE(target->getInstanceMaterials().empty());
}

using StaticGeometryLayout = access::Layout<StaticGeometryComponent>;

namespace {
	// The material count is the first stream field; a guid-less stream cannot be built from a component without assets.
	[[nodiscard]] Vector<std::byte> payloadWithMaterialCount(const u64 count_) {
		const auto source = make<StaticGeometryComponent>();
		Vector<std::byte> bytes {};
		::hg::engine::acs::dump_payload(*source, bytes);
		std::memcpy(bytes.data(), &count_, sizeof(count_));
		return bytes;
	}
}

TEST(StaticGeometrySerialization, TrailingNoneMaterialSlotsSurvive) {
	const auto bytes = payloadWithMaterialCount(3uLL);
	const auto target = make<StaticGeometryComponent>();
	ASSERT_EQ(::hg::engine::acs::check_payload<StaticGeometryComponent>(bytes, 0uLL), ~0uLL);
	::hg::engine::acs::commit_payload(bytes, 0uLL, *target);
	LoadContext ctx { bytes.size() };
	::hg::engine::acs::resolve_payload(bytes, 0uLL, *target, ctx);
	EXPECT_EQ(target->getInstanceMaterials().size(), 3uLL);
	for (const auto& material : target->getInstanceMaterials()) {
		EXPECT_FALSE(material.has_value());
	}

	Vector<std::byte> again {};
	::hg::engine::acs::dump_payload(*target, again);
	EXPECT_EQ(again, bytes);
}

TEST(StaticGeometrySerialization, MaterialCountOverCapacityRejected) {
	const auto bytes = payloadWithMaterialCount(17uLL);
	EXPECT_EQ(::hg::engine::acs::check_payload<StaticGeometryComponent>(bytes, 0uLL), 0uLL);
	EXPECT_NE(::hg::engine::acs::check_payload<StaticGeometryComponent>(payloadWithMaterialCount(16uLL), 0uLL), 0uLL);
}

TEST(StaticGeometrySerialization, MaterialGuidPastCountRejected) {
	auto bytes = payloadWithMaterialCount(2uLL);
	const u64 guidsAt = bytes.size() - StaticGeometryLayout::assets * sizeof(AssetGuid);
	ASSERT_EQ(::hg::engine::acs::check_payload<StaticGeometryComponent>(bytes, 0uLL), ~0uLL);

	// Guid index 3 is a material slot past the count of 2 (index 0 is the geometry).
	const u64 stray = guidsAt + 3uLL * sizeof(AssetGuid);
	bytes[stray] = std::byte { 0x5A };
	EXPECT_EQ(::hg::engine::acs::check_payload<StaticGeometryComponent>(bytes, 0uLL), stray);

	// Within the count it stays acceptable; resolveAsset reports what it cannot find.
	auto inside = payloadWithMaterialCount(2uLL);
	inside[guidsAt + 2uLL * sizeof(AssetGuid)] = std::byte { 0x5A };
	EXPECT_EQ(::hg::engine::acs::check_payload<StaticGeometryComponent>(inside, 0uLL), ~0uLL);
}

namespace {
	[[nodiscard]] Opt<ReadError> hydrateWithMaterialSlots(cref<AssetGuid> geometry_, const u64 count_) {
		auto archive = BufferArchive {}; {
			const auto source = make<StaticGeometryComponent>();
			auto structured = StructuredArchive { archive };
			auto record = structured.insertRootSlot().intoStruct();
			access::Structure<Guid>::serialize(geometry_, record.insertStructSlot("geometry"sv));
			{
				auto materials = record.insertRecordSlot("materials"sv).intoSeq();
				for (u64 i = 0uLL; i < count_; ++i) {
					access::Structure<Guid>::serialize(invalid_asset_guid, materials.addRecordSlot().intoStruct());
				}
			}
			access::Structure<HierarchyComponent>::serialize(*source, ::hg::move(record));
		}
		archive.seek(0);

		auto ctx = LoadContext { static_cast<u64>(archive.totalSize()) };
		const auto target = make<StaticGeometryComponent>(); {
			const auto root = RootScopedSlot { archive, ScopedSlotStateFlag::eClean, &ctx };
			access::Structure<StaticGeometryComponent>::hydrate(root.asStruct(), *target);
		}
		EXPECT_TRUE(target->getInstanceMaterials().empty());
		return ctx.firstError() != nullptr ? Opt<ReadError> { *ctx.firstError() } : Opt<ReadError> {};
	}

	// An unresolved geometry skips its materials without an error, so only the capacity check can reject a slot.
	const AssetGuid unresolvedGeometry { 1uL, 2u, 3u, 4uLL };
}

TEST(StaticGeometrySerialization, MaterialIndexWithoutGeometryRejected) {
	const auto error = hydrateWithMaterialSlots(invalid_asset_guid, 1uLL);
	ASSERT_TRUE(error.has_value());
	EXPECT_EQ(error->kind, ReadError::Kind::eInvalidValue);
	EXPECT_EQ(error->detail, "instance material index is out of range of the static geometry");
}

TEST(StaticGeometrySerialization, MaterialSlotsUpToCapacityAccepted) {
	EXPECT_FALSE(hydrateWithMaterialSlots(unresolvedGeometry, StaticGeometryLayout::material_capacity).has_value());
}

TEST(StaticGeometrySerialization, MaterialIndexAtCapacityRejected) {
	const auto error = hydrateWithMaterialSlots(unresolvedGeometry, StaticGeometryLayout::material_capacity + 1uLL);
	ASSERT_TRUE(error.has_value());
	EXPECT_EQ(error->kind, ReadError::Kind::eInvalidValue);
	EXPECT_EQ(error->detail, "instance material index exceeds the material capacity");
}
