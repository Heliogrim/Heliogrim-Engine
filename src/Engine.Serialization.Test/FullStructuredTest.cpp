#include "pch.h"

/**/
#include <cstring>
#include <Engine.Assets.Type/Asset.hpp>
#include <Engine.Assets.Type/Texture/TextureAsset.hpp>
#include <Engine.Assets.Type/Universe/LevelAsset.hpp>

/**/
#include <Engine.Serialization.Structures/Symbols.hpp>
#include <Engine.Serialization.Structures/Common/Guid.hpp>

import Engine.Common;
import Engine.Resource.Archive;
import Engine.Serialization;

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
	mref<StructScopedSlot> record_
) {
	record_.insertSlot<float>("x") << self_.x;
	record_.insertSlot<float>("y") << self_.y;
	record_.insertSlot<float>("z") << self_.z;
}

template <>
void access::Structure<SerializationModule::TrivialTestClass>::hydrate(
	cref<StructScopedSlot> record_,
	ref<SerializationModule::TrivialTestClass> target_
) {
	record_.getSlot<float>("x") >> target_.x;
	record_.getSlot<float>("y") >> target_.y;
	record_.getSlot<float>("z") >> target_.z;
}

template <>
void access::Structure<SerializationModule::FlatTestClass>::serialize(
	const SerializationModule::FlatTestClass& self_,
	mref<StructScopedSlot> record_
) {
	record_.insertSlot<String>("name") << self_.name;
	Structure<SerializationModule::TrivialTestClass>::serialize(self_.subclass, record_.insertStructSlot("subclass"));
}

template <>
void access::Structure<SerializationModule::FlatTestClass>::hydrate(
	cref<StructScopedSlot> record_,
	ref<SerializationModule::FlatTestClass> target_
) {
	record_.getSlot<String>("name") >> target_.name;
	Structure<SerializationModule::TrivialTestClass>::hydrate(record_.getStructSlot("subclass"), target_.subclass);
}

template <>
void access::Structure<SerializationModule::FlatTestWithArgsClass>::serialize(
	const SerializationModule::FlatTestWithArgsClass& self_,
	mref<StructScopedSlot> record_
) {
	// Note: Will be supplied by the constructor arguments
	// slot.insertSlot<String>("name") << self_->name;
	Structure<SerializationModule::TrivialTestClass>::serialize(self_.subclass, record_.insertStructSlot("subclass"));
}

template <>
void access::Structure<SerializationModule::FlatTestWithArgsClass>::hydrate(
	cref<StructScopedSlot> record_,
	ref<SerializationModule::FlatTestWithArgsClass> target_
) {
	// Note: Will be supplied by the constructor arguments
	// slot.getSlot<String>("name") >> target_.name;
	Structure<SerializationModule::TrivialTestClass>::hydrate(record_.getStructSlot("subclass"), target_.subclass);
}

template <>
void access::Structure<SerializationModule::NestedTestClass>::serialize(
	const SerializationModule::NestedTestClass& self_,
	mref<StructScopedSlot> record_
) {
	record_.insertSlot<String>("data") << self_.data;
	Structure<SerializationModule::TrivialTestClass>::serialize(*self_.nested, record_.insertStructSlot("nested"));
}

template <>
void access::Structure<SerializationModule::NestedTestClass>::hydrate(
	cref<StructScopedSlot> record_,
	ref<SerializationModule::NestedTestClass> target_
) {
	record_.getSlot<String>("data") >> target_.data;
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
	mref<StructScopedSlot> record_
) {
	// Note: Will be supplied by the constructor arguments
	// slot.insertSlot<String>("data") << self_->data;
	Structure<SerializationModule::FlatTestWithArgsClass>::serialize(*self_.nested, record_.insertStructSlot("nested"));
}

template <>
void access::Structure<SerializationModule::NestedTestWithArgsClass>::hydrate(
	cref<StructScopedSlot> record_,
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

namespace SerializationModule {
	TEST(FullStructuredTest, ShortGuidStringRejected) {

		auto archive = BufferArchive {};
		{
			auto structured = StructuredArchive { archive };
			auto record = structured.insertRootSlot().intoStruct();
			record.insertSlot<String>(Symbols::Guid) << String { "abcd" };
			record.insertSlot<String>("named") << String { "abcd" };
		}

		archive.seek(0);

		{
			auto structured = StructuredArchive { archive };
			const auto record = structured.getRootSlot().asStruct();
			EXPECT_EQ(get_guid_slot(record), Guid {});
			// Context-less: rejection is reported through LoadContext only (LoadBoundary.ShortActorGuidRejected).
		}

		archive.clearError();
		archive.seek(0);

		{
			auto structured = StructuredArchive { archive };
			const auto record = structured.getRootSlot().asStruct();
			EXPECT_EQ(get_named_guid_slot(record, "named"), Guid {});
			// Context-less: rejection is reported through LoadContext only (LoadBoundary.ShortActorGuidRejected).
		}
	}
}

namespace SerializationModule {
	TEST(FullStructuredTest, LevelAssetBadLevelDataRejected) {
		for (const auto bad : { "file://nope", "archive://zz", "archive://99999999999999999999", "archive://1-2-3" }) {

			auto archive = BufferArchive {};
			{
				auto structured = StructuredArchive { archive };
				auto record = structured.insertRootSlot().intoStruct();
				record.insertSlot<String>("levelData") << String { bad };
				access::Structure<engine::assets::Asset>::serialize(engine::assets::LevelAsset {}, ::hg::move(record));
			}

			archive.seek(0);

			engine::assets::LevelAsset asset {};
			{
				auto structured = StructuredArchive { archive };
				access::Structure<engine::assets::LevelAsset>::hydrate(structured.getRootSlot().asStruct(), asset);
			}

			// Context-less: rejection is reported through LoadContext only (LoadBoundary.LevelAssetBadLevelDataRejected).
			// A rejected url leaves the default (null) archive url.
			ASSERT_TRUE(asset.getLevelData().is<engine::storage::ArchiveUrl>()) << bad;
			EXPECT_EQ(asset.getLevelData().as<engine::storage::ArchiveUrl>().guid(), engine::resource::ArchiveGuid {}) << bad;
		}
	}
}

namespace SerializationModule {
	TEST(FullStructuredTest, TextureAssetInvalidImageCountIgnored) {

		auto archive = BufferArchive {};
		{
			auto source = engine::assets::TextureAsset {};
			source.addSourceImage(AssetGuid { 1uL, 2u, 3u, 4uLL });
			auto structured = StructuredArchive { archive };
			access::Structure<engine::assets::TextureAsset>::serialize(source, structured.insertRootSlot().intoStruct());
		}

		// Layout after the key: seq type (1 byte), size (8 bytes), count (8 bytes); ~0 reads back as count -1.
		constexpr auto key = "images"sv;
		const auto text = std::string_view { reinterpret_cast<const char*>(archive.data()), archive.size() };
		const auto at = text.find(key);
		ASSERT_NE(at, std::string_view::npos);
		const auto seqAt = at + key.size();
		ASSERT_EQ(archive[seqAt], static_cast<_::byte>(StructureSlotType::eSeq));
		constexpr auto invalidCount = ~0uLL;
		std::memcpy(&archive[seqAt + 9uLL], &invalidCount, sizeof(invalidCount));

		archive.seek(0);

		engine::assets::TextureAsset target {};
		{
			auto structured = StructuredArchive { archive };
			EXPECT_NO_THROW(access::Structure<engine::assets::TextureAsset>::hydrate(structured.getRootSlot().asStruct(), target));
		}
		EXPECT_FALSE(target.hasSourceImage(AssetGuid { 1uL, 2u, 3u, 4uLL }));
	}
}
