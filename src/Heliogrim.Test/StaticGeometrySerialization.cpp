#include "pch.h"

/**/

#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Resource.Archive/BufferArchive.hpp>
#include <Engine.Serialization/Access/Structure.hpp>
#include <Engine.Serialization/Archive/StructuredArchive.hpp>
#include <Engine.Serialization/Structure/StructScopedSlot.hpp>
#include <Heliogrim/Component/Scene/Geometry/StaticGeometryComponent.Layout.hpp>

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

TEST(StaticGeometrySerialization, LayoutOverCountRecovers) {
	const auto source = make<StaticGeometryComponent>();
	Vector<std::byte> bytes {};
	access::Layout<StaticGeometryComponent>::dump(*source, bytes);

	// Overwrite the material count after the geometry guid with an out-of-range value.
	const auto count = access::Layout<StaticGeometryComponent>::material_capacity + 1uLL;
	std::memcpy(bytes.data() + sizeof(AssetGuid), &count, sizeof(count));

	const auto target = make<StaticGeometryComponent>();
	access::Layout<StaticGeometryComponent>::load(bytes, *target);
	EXPECT_TRUE(target->getInstanceMaterials().empty());
}
