#include "pch.h"

/**/

#include <Engine.Serialization.Structures/Common/Guid.hpp>
#include <Engine.Graphics.Components/Geometry/BoxShapeGeometryComponent.Layout.hpp>
#include <Engine.Graphics.Components/Geometry/SphereShapeGeometryComponent.Layout.hpp>
#include <Engine.Physics.Components/Collision/BoxShapeCollisionComponent.Layout.hpp>
#include <Engine.Physics.Components/Collision/SphereShapeCollisionComponent.Layout.hpp>
#include <Heliogrim/Actor/Archetype/ArchetypeLayout.inl.hpp>

import Engine.Common;
import Engine.Resource.Archive;
import Engine.Serialization;

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

TEST(ShapeComponentSerialization, BoxGeometryRoundTrip) {
	const auto source = make<BoxShapeGeometryComponent>();
	source->setExtent(math::Vector3f32 { 1.F, 2.F, 3.F });
	for (const auto fn : { &layoutRoundTrip<BoxShapeGeometryComponent>, &structuredRoundTrip<BoxShapeGeometryComponent> }) {
		const auto target = make<BoxShapeGeometryComponent>();
		fn(*source, *target);
		EXPECT_EQ(target->getExtent(), source->getExtent());
		EXPECT_EQ(target->getInstanceMaterial(), None);
	}
}

TEST(ShapeComponentSerialization, SphereGeometryRoundTrip) {
	const auto source = make<SphereShapeGeometryComponent>();
	source->setRadius(math::Vector1f32 { 4.F });
	for (const auto fn : { &layoutRoundTrip<SphereShapeGeometryComponent>, &structuredRoundTrip<SphereShapeGeometryComponent> }) {
		const auto target = make<SphereShapeGeometryComponent>();
		fn(*source, *target);
		EXPECT_EQ(target->getRadius().at<0>(), 4.F);
		EXPECT_EQ(target->getInstanceMaterial(), None);
	}
}

TEST(ShapeComponentSerialization, BoxCollisionRoundTrip) {
	const auto source = make<BoxShapeCollisionComponent>();
	source->settings = { .isStatic = true, .isTrigger = false };
	source->extent = math::Vector3f32 { 5.F, 6.F, 7.F };
	for (const auto fn : { &layoutRoundTrip<BoxShapeCollisionComponent>, &structuredRoundTrip<BoxShapeCollisionComponent> }) {
		const auto target = make<BoxShapeCollisionComponent>();
		target->settings = { .isStatic = false, .isTrigger = true };
		fn(*source, *target);
		EXPECT_TRUE(target->settings.isStatic);
		EXPECT_FALSE(target->settings.isTrigger);
		EXPECT_EQ(target->extent, source->extent);
	}
}

TEST(ShapeComponentSerialization, SphereCollisionRoundTrip) {
	const auto source = make<SphereShapeCollisionComponent>();
	source->settings = { .isStatic = false, .isTrigger = true };
	source->radius = math::Vector1f32 { 8.F };
	for (const auto fn : { &layoutRoundTrip<SphereShapeCollisionComponent>, &structuredRoundTrip<SphereShapeCollisionComponent> }) {
		const auto target = make<SphereShapeCollisionComponent>();
		target->settings = { .isStatic = true, .isTrigger = false };
		fn(*source, *target);
		EXPECT_FALSE(target->settings.isStatic);
		EXPECT_TRUE(target->settings.isTrigger);
		EXPECT_EQ(target->radius.at<0>(), 8.F);
	}
}

TEST(ShapeComponentSerialization, MaterialGuidWithoutEngineIsUnresolvedWarning) {
	auto archive = BufferArchive {}; {
		const auto source = make<BoxShapeGeometryComponent>();
		auto structured = StructuredArchive { archive };
		auto record = structured.insertRootSlot().intoStruct();
		access::Structure<math::fvec3>::serialize(math::fvec3 { 1.F }, record.insertStructSlot("extent"));
		access::Structure<Guid>::serialize(AssetGuid { 1uL, 2u, 3u, 4uLL }, record.insertStructSlot("material"));
		access::Structure<HierarchyComponent>::serialize(*source, ::hg::move(record));
	}
	archive.seek(0);

	auto ctx = LoadContext { static_cast<u64>(archive.totalSize()) };
	const auto target = make<BoxShapeGeometryComponent>(); {
		const auto root = RootScopedSlot { archive, ScopedSlotStateFlag::eClean, &ctx };
		access::Structure<BoxShapeGeometryComponent>::hydrate(root.asStruct(), *target);
	}

	EXPECT_FALSE(ctx.failed());
	EXPECT_EQ(target->getInstanceMaterial(), None);
	engine::diag::CollectingSink sink {};
	ASSERT_TRUE(ctx.finish(sink, "box").has_value());
	ASSERT_EQ(sink.items().size(), 1uLL);
	EXPECT_EQ(sink.items()[0].code()->key, "unresolved-reference");
}
