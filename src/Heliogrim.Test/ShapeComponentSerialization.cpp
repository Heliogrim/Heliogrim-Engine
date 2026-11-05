#include "pch.h"

/**/

#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Resource.Archive/BufferArchive.hpp>
#include <Engine.Serialization/Access/Structure.hpp>
#include <Engine.Serialization/Archive/StructuredArchive.hpp>
#include <Engine.Serialization/Structure/StructScopedSlot.hpp>
#include <Heliogrim/Component/Scene/Collision/BoxShapeCollisionComponent.Layout.hpp>
#include <Heliogrim/Component/Scene/Collision/SphereShapeCollisionComponent.Layout.hpp>
#include <Heliogrim/Component/Scene/Geometry/BoxShapeGeometryComponent.Layout.hpp>
#include <Heliogrim/Component/Scene/Geometry/SphereShapeGeometryComponent.Layout.hpp>

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
		access::Layout<Type_>::dump(source_, bytes);
		ASSERT_EQ(bytes.size(), access::Layout<Type_>::size);
		access::Layout<Type_>::load(bytes, target_);
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
