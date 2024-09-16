#include "pch.h"

/**/
#include <cstring>
#include <list>
#include <tuple>
#include <utility>
#include <Engine.Assets.Type/Asset.hpp>
#include <Engine.Common/Collection/Array.hpp>
#include <Engine.Resource.Archive/BufferArchive.hpp>

/**/
#include <Engine.Serialization/Access/Structure.hpp>
#include <Engine.Serialization/Structure/FloatScopedSlot.hpp>
#include <Engine.Serialization/Structure/IntegralScopedSlot.hpp>
#include <Engine.Serialization/Structure/MapEntryScopedSlot.hpp>
#include <Engine.Serialization/Structure/MapScopedSlot.hpp>
#include <Engine.Serialization/Structure/ScopedStructureSlot.hpp>
#include <Engine.Serialization/Structure/SeqScopedSlot.hpp>
#include <Engine.Serialization/Structure/SliceScopedSlot.hpp>
#include <Engine.Serialization/Structure/StringScopedSlot.hpp>
#include <Engine.Serialization/Structure/StructScopedSlot.hpp>

#include "Engine.Serialization/Archive/StructuredArchive.hpp"
#include "Engine.Serialization/Scheme/DefaultScheme.hpp"

using namespace hg::engine::resource;
using namespace hg::engine::serialization;
using namespace hg;

namespace SerializationModule {
	struct TrivialTestClass {
		float x, y, z;
	};

	struct FlatTestClass {
		String name;
		TrivialTestClass subclass;
	};

	struct FlatTestWithArgsClass {
		FlatTestWithArgsClass() = delete;

		explicit FlatTestWithArgsClass(mref<String> name_) :
			name(std::move(name_)),
			subclass() {}

		FlatTestWithArgsClass(cref<FlatTestWithArgsClass>) noexcept = default;

		String name;
		TrivialTestClass subclass;
	};

	struct NestedTestClass {
		String data;
		ptr<TrivialTestClass> nested;
	};

	struct NestedTestWithArgsClass {
		NestedTestWithArgsClass() = delete;

		explicit NestedTestWithArgsClass(mref<String> data_) :
			data(std::move(data_)),
			nested(nullptr) {}

		NestedTestWithArgsClass(cref<NestedTestWithArgsClass>) noexcept = default;

		String data;
		ptr<FlatTestWithArgsClass> nested;
	};
}

/**/

template <>
void access::Structure<SerializationModule::TrivialTestClass>::serialize(
	const SerializationModule::TrivialTestClass& self_,
	mref<StructScopedSlot> slot_
) {
	slot_.insertSlot<float>("x") << self_.x;
	slot_.insertSlot<float>("y") << self_.y;
	slot_.insertSlot<float>("z") << self_.z;
}

template <>
void access::Structure<SerializationModule::TrivialTestClass>::hydrate(
	cref<StructScopedSlot> slot_,
	ref<SerializationModule::TrivialTestClass> target_
) {
	slot_.getSlot<float>("x") >> target_.x;
	slot_.getSlot<float>("y") >> target_.y;
	slot_.getSlot<float>("z") >> target_.z;
}

template <>
void access::Structure<SerializationModule::FlatTestClass>::serialize(
	const SerializationModule::FlatTestClass& self_,
	mref<StructScopedSlot> slot_
) {
	slot_.insertSlot<String>("name") << self_.name;
	Structure<SerializationModule::TrivialTestClass>::serialize(self_.subclass, slot_.insertStructSlot("subclass"));
}

template <>
void access::Structure<SerializationModule::FlatTestClass>::hydrate(
	cref<StructScopedSlot> slot_,
	ref<SerializationModule::FlatTestClass> target_
) {
	slot_.getSlot<String>("name") >> target_.name;
	Structure<SerializationModule::TrivialTestClass>::hydrate(slot_.getStructSlot("subclass"), target_.subclass);
}

template <>
void access::Structure<SerializationModule::FlatTestWithArgsClass>::serialize(
	const SerializationModule::FlatTestWithArgsClass& self_,
	mref<StructScopedSlot> slot_
) {
	// Note: Will be supplied by the constructor arguments
	// slot.insertSlot<String>("name") << self_->name;
	Structure<SerializationModule::TrivialTestClass>::serialize(self_.subclass, slot_.insertStructSlot("subclass"));
}

template <>
void access::Structure<SerializationModule::FlatTestWithArgsClass>::hydrate(
	cref<StructScopedSlot> slot_,
	ref<SerializationModule::FlatTestWithArgsClass> target_
) {
	// Note: Will be supplied by the constructor arguments
	// slot.getSlot<String>("name") >> target_.name;
	Structure<SerializationModule::TrivialTestClass>::hydrate(slot_.getStructSlot("subclass"), target_.subclass);
}

template <>
void access::Structure<SerializationModule::NestedTestClass>::serialize(
	const SerializationModule::NestedTestClass& self_,
	mref<StructScopedSlot> slot_
) {
	slot_.insertSlot<String>("data") << self_.data;
	Structure<SerializationModule::TrivialTestClass>::serialize(*self_.nested, slot_.insertStructSlot("nested"));
}

template <>
void access::Structure<SerializationModule::NestedTestClass>::hydrate(
	cref<StructScopedSlot> slot_,
	ref<SerializationModule::NestedTestClass> target_
) {
	slot_.getSlot<String>("data") >> target_.data;
}

template <>
template <>
decltype(auto) DefaultScheme<SerializationModule::NestedTestClass>::deserialize<
	access::Structure<SerializationModule::NestedTestClass>
>(
	_In_ cref<RecordScopedSlot> slot_,
	NewDeleteAllocator&& allocator_
) {
	using access_type = access::Structure<SerializationModule::NestedTestClass>;

	auto* target = access_type::instantiate(allocator_);
	access_type::hydrate(slot_.asStruct(), *target);

	target->nested = access::Structure<SerializationModule::TrivialTestClass>::deserialize(
		slot_.asStruct().getRecordSlot("nested"),
		allocator_
	);

	return target;
}

template <>
void access::Structure<SerializationModule::NestedTestWithArgsClass>::serialize(
	const SerializationModule::NestedTestWithArgsClass& self_,
	mref<StructScopedSlot> slot_
) {
	// Note: Will be supplied by the constructor arguments
	// slot.insertSlot<String>("data") << self_->data;
	Structure<SerializationModule::FlatTestWithArgsClass>::serialize(*self_.nested, slot_.insertStructSlot("nested"));
}

template <>
void access::Structure<SerializationModule::NestedTestWithArgsClass>::hydrate(
	cref<StructScopedSlot> slot_,
	ref<SerializationModule::NestedTestWithArgsClass> target_
) {
	// Note: Will be supplied by the constructor arguments
	// slot.getSlot<String>("data") >> target_.data;
}

template <>
template <>
decltype(auto) DefaultScheme<
	SerializationModule::NestedTestWithArgsClass
>::deserialize<access::Structure<SerializationModule::NestedTestWithArgsClass>>(
	_In_ cref<RecordScopedSlot> slot_,
	NewDeleteAllocator&& allocator_,
	mref<String> data_,
	mref<String> name_
) {
	using access_type = access::Structure<SerializationModule::NestedTestWithArgsClass>;

	auto* target = access_type::instantiate(
		allocator_,
		std::move(data_)
	);
	access_type::hydrate(slot_.asStruct(), *target);

	target->nested = access::Structure<SerializationModule::FlatTestWithArgsClass>::deserialize(
		slot_.asStruct().getRecordSlot("nested"),
		allocator_,
		std::move(name_)
	);

	return target;
}

/**/

namespace SerializationModule {
	TEST(FullStructuredTest, TrivialReconstruct) {

		auto archive = BufferArchive {};
		constexpr TrivialTestClass data {
			.x = 236757726.F,
			.y = 0.34683762F,
			.z = 3256.32423F
		};

		/**/

		{
			auto structured = StructuredArchive { archive };
			constexpr TrivialTestClass source { data };

			::hg::engine::serialization::access::Structure<TrivialTestClass>::serialize(
				source,
				structured.insertRootSlot().intoStruct()
			);
		}

		/**/

		archive.seek(0);

		/**/

		{
			auto structured = StructuredArchive { archive };
			TrivialTestClass restored {};

			::hg::engine::serialization::access::Structure<TrivialTestClass>::hydrate(
				structured.getRootSlot().asStruct(),
				restored
			);

			/**/

			EXPECT_EQ(restored.x, data.x);
			EXPECT_EQ(restored.y, data.y);
			EXPECT_EQ(restored.z, data.z);
		}

	}

	TEST(FullStructuredTest, FlatReconstruct) {

		auto archive = BufferArchive {};
		const FlatTestClass data {
			.name = "Hello World",
			.subclass = TrivialTestClass {
				.x = 46376.F,
				.y = 0.25762F,
				.z = 5235.32F
			}
		};

		/**/

		{
			auto structured = StructuredArchive { archive };
			const FlatTestClass source { data };

			::hg::engine::serialization::access::Structure<FlatTestClass>::serialize(
				source,
				structured.insertRootSlot().intoStruct()
			);
		}

		/**/

		archive.seek(0);

		/**/

		{
			auto structured = StructuredArchive { archive };
			auto* restored = ::hg::engine::serialization::access::Structure<FlatTestClass>::deserialize(
				structured.getRootSlot(),
				NewDeleteAllocator {}
			);

			/**/

			ASSERT_NE(restored, nullptr);
			EXPECT_EQ(restored->name, data.name);
			EXPECT_EQ(restored->subclass.x, data.subclass.x);
			EXPECT_EQ(restored->subclass.y, data.subclass.y);
			EXPECT_EQ(restored->subclass.z, data.subclass.z);

			delete restored;
		}

	}

	TEST(FullStructuredTest, FlatReconstructWithArgs) {

		auto archive = BufferArchive {};
		const auto data = [] {
			auto tmp = FlatTestWithArgsClass { "Hello World" };
			tmp.subclass = TrivialTestClass {
				.x = 46376.F,
				.y = 0.25762F,
				.z = 5235.32F
			};
			return tmp;
		}();

		/**/

		{
			auto structured = StructuredArchive { archive };
			const FlatTestWithArgsClass source { data };

			::hg::engine::serialization::access::Structure<FlatTestWithArgsClass>::serialize(
				source,
				structured.insertRootSlot().intoStruct()
			);
		}

		/**/

		archive.seek(0);

		/**/

		{
			auto structured = StructuredArchive { archive };
			auto* restored = ::hg::engine::serialization::access::Structure<FlatTestWithArgsClass>::deserialize(
				structured.getRootSlot(),
				NewDeleteAllocator {},
				clone(data.name)
			);

			/**/

			ASSERT_NE(restored, nullptr);
			EXPECT_EQ(restored->name, data.name);
			EXPECT_EQ(restored->subclass.x, data.subclass.x);
			EXPECT_EQ(restored->subclass.y, data.subclass.y);
			EXPECT_EQ(restored->subclass.z, data.subclass.z);

			delete restored;
		}
	}

	TEST(FullStructuredTest, NestedReconstruct) {

		auto archive = BufferArchive {};
		const auto holder = make_uptr<TrivialTestClass>(46376.F, 0.25762F, 5235.32F);
		const NestedTestClass data {
			.data = "Hello World",
			.nested = holder.get()
		};

		/**/

		{
			auto structured = StructuredArchive { archive };
			const NestedTestClass source { data };

			::hg::engine::serialization::access::Structure<NestedTestClass>::serialize(
				source,
				structured.insertRootSlot().intoStruct()
			);
		}

		/**/

		archive.seek(0);

		/**/

		{
			auto structured = StructuredArchive { archive };
			auto* restored = ::hg::engine::serialization::access::Structure<NestedTestClass>::deserialize(
				structured.getRootSlot(),
				NewDeleteAllocator {}
			);

			/**/

			ASSERT_NE(restored, nullptr);
			EXPECT_EQ(restored->data, data.data);

			ASSERT_NE(restored->nested, nullptr);
			EXPECT_EQ(restored->nested->x, data.nested->x);
			EXPECT_EQ(restored->nested->y, data.nested->y);
			EXPECT_EQ(restored->nested->z, data.nested->z);

			delete std::exchange(restored->nested, nullptr);
			delete restored;
		}

	}

	TEST(FullStructuredTest, NestedReconstructWithArgs) {

		auto archive = BufferArchive {};
		const auto holder = make_uptr<FlatTestWithArgsClass>(
			[] {
				auto tmp = FlatTestWithArgsClass { "Hello World" };
				tmp.subclass = TrivialTestClass {
					.x = 46376.F,
					.y = 0.25762F,
					.z = 5235.32F
				};
				return tmp;
			}()
		);
		const auto source = [nested = holder.get()] {
			auto tmp = NestedTestWithArgsClass { "Hello World" };
			tmp.nested = nested;
			return tmp;
		}();

		/**/

		{
			auto structured = StructuredArchive { archive };
			::hg::engine::serialization::access::Structure<NestedTestWithArgsClass>::serialize(
				source,
				structured.insertRootSlot().intoStruct()
			);
		}

		/**/

		archive.seek(0);

		/**/

		{
			auto structured = StructuredArchive { archive };
			auto* restored = ::hg::engine::serialization::access::Structure<NestedTestWithArgsClass>::deserialize(
				structured.getRootSlot(),
				NewDeleteAllocator {},
				clone(source.data),
				clone(source.nested->name)
			);

			/**/

			ASSERT_NE(restored, nullptr);
			EXPECT_EQ(restored->data, source.data);

			ASSERT_NE(restored->nested, nullptr);
			EXPECT_EQ(restored->nested->name, source.nested->name);

			EXPECT_EQ(restored->nested->subclass.x, source.nested->subclass.x);
			EXPECT_EQ(restored->nested->subclass.y, source.nested->subclass.y);
			EXPECT_EQ(restored->nested->subclass.z, source.nested->subclass.z);

			delete std::exchange(restored->nested, nullptr);
			delete restored;
		}
	}
}
