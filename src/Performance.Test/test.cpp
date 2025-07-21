#include "pch.h"

/**/
#include <chrono>
#include <cstring>
#include <format>
#include <list>
#include <tuple>
#include <utility>
#include <Engine.Assets.Type/Asset.hpp>
#include <Engine.Common/Collection/Array.hpp>
#include <Engine.Resource.Archive/BufferArchive.hpp>
#include <Engine.Serialization/Archive/LayoutArchive.hpp>
#include <Engine.Serialization/Archive/StructuredArchive.hpp>
#include <Engine.Serialization/Layout/DataLayout.hpp>
#include <Engine.Serialization/Layout/LayoutDefine.hpp>
#include <Engine.Serialization/Layout/LayoutDefineSpan.hpp>
#include <Engine.Serialization/Layout/LayoutDefineValue.hpp>

using namespace hg::engine::resource;
using namespace hg::engine::serialization;
using namespace hg;

namespace SerializationModule {
	struct TestSubTypePayload {
		u64 p0;
		u64 p1;
		float f0;
		float f1;
	};

	class TestSerialSubTypeVectorizedSliceAsset :
		public InheritMeta<TestSerialSubTypeVectorizedSliceAsset, ::hg::engine::assets::Asset> {
	public:
		template <typename>
		friend class ::hg::engine::serialization::DataLayout;

	public:
		inline static constexpr AssetTypeId typeId { "TestSerialSubTypeVectorizedSliceAsset"_typeId };

	public:
		TestSerialSubTypeVectorizedSliceAsset() :
			InheritMeta(
				AssetGuid {},
				typeId,
				"TestSerialSubTypeVectorizedSliceAsset"sv,
				engine::assets::AssetReferenceUrl {},
				engine::assets::AssetUrl {}
			) {}

	public:
		Vector<TestSubTypePayload> payload;
	};
}

namespace hg::engine::serialization {
	template <>
	void DataLayout<SerializationModule::TestSubTypePayload>::describe() {

		using namespace ::SerializationModule;
		using namespace ::hg::engine::serialization::layout;

		// TODO: make define for sub objects
		defineValue<LayoutDefineValueType::eUInt64>(offsetof(TestSubTypePayload, p0));
		defineValue<LayoutDefineValueType::eUInt64>(offsetof(TestSubTypePayload, p1));
		defineValue<LayoutDefineValueType::eFloat>(offsetof(TestSubTypePayload, f0));
		defineValue<LayoutDefineValueType::eFloat>(offsetof(TestSubTypePayload, f1));
	}

	template <>
	void DataLayout<SerializationModule::TestSerialSubTypeVectorizedSliceAsset>::describe() {

		using namespace ::SerializationModule;
		using namespace ::hg::engine::serialization::layout;

		const auto subLayout { make_sptr<DataLayout<TestSubTypePayload>>() };
		subLayout->reflect().storeType<TestSubTypePayload>();
		subLayout->describe();

		// TODO: make define for sub objects
		defineSlice<Vector<TestSubTypePayload>>(offsetof(TestSerialSubTypeVectorizedSliceAsset, payload), subLayout);
	}
}

namespace SerializationModule {
	TEST(LayoutArchive, AutoVectorizedSliceSubType) {

		BufferArchive archive {};
		DataLayout<TestSerialSubTypeVectorizedSliceAsset> layout {};
		layout.describe();

		TypedLayoutArchive<TestSerialSubTypeVectorizedSliceAsset> arch { archive, &layout };

		auto writeAsset = new TestSerialSubTypeVectorizedSliceAsset();
		//writeAsset->payload.resize(200'000'000, { 1231uLL, 738956uLL, 2.32906F, 9230.35F });
		writeAsset->payload.resize(2'000'000, { 1231uLL, 738956uLL, 2.32906F, 9230.35F });

		const auto writeStart { std::chrono::high_resolution_clock::now() };
		arch << writeAsset;
		const auto writeEnd { std::chrono::high_resolution_clock::now() };

		archive.seek(0);

		auto readAsset = new TestSerialSubTypeVectorizedSliceAsset();

		const auto readStart { std::chrono::high_resolution_clock::now() };
		arch >> readAsset;
		const auto readEnd { std::chrono::high_resolution_clock::now() };

		EXPECT_EQ(writeAsset->payload.size(), readAsset->payload.size());
		EXPECT_EQ(writeAsset->payload.capacity(), readAsset->payload.capacity());

		/**/

		const auto memCpyStart { std::chrono::high_resolution_clock::now() };
		Vector<TestSubTypePayload> memCpyTarget {};
		memCpyTarget.resize(writeAsset->payload.size());
		std::memcpy(
			memCpyTarget.data(),
			writeAsset->payload.data(),
			writeAsset->payload.size() * sizeof(TestSubTypePayload)
		);
		const auto memCpyEnd { std::chrono::high_resolution_clock::now() };

		/**/

		memCpyTarget.clear();
		memCpyTarget.shrink_to_fit();

		/**/

		const auto cpyStart { std::chrono::high_resolution_clock::now() };
		memCpyTarget.reserve(writeAsset->payload.size());
		for (const auto& entry : writeAsset->payload) {
			memCpyTarget.emplace_back(entry);
		}
		const auto cpyEnd { std::chrono::high_resolution_clock::now() };

		/**/

		std::cout << "[Auto] " << std::format(
			"Serialization: {:}ms ({:}ns)",
			std::chrono::duration_cast<std::chrono::milliseconds>(writeEnd - writeStart).count(),
			std::chrono::duration_cast<std::chrono::nanoseconds>(writeEnd - writeStart).count() /
			writeAsset->payload.size()
		) << std::format(
			" | Deserialization: {:}ms ({:}ns)",
			std::chrono::duration_cast<std::chrono::milliseconds>(readEnd - readStart).count(),
			std::chrono::duration_cast<std::chrono::nanoseconds>(readEnd - readStart).count() /
			writeAsset->payload.size()
		) << std::format(
			" | MemCpy: {:}ms ({:}ns)",
			std::chrono::duration_cast<std::chrono::milliseconds>(memCpyEnd - memCpyStart).count(),
			std::chrono::duration_cast<std::chrono::nanoseconds>(memCpyEnd - memCpyStart).count() /
			writeAsset->payload.size()
		) << std::format(
			" | Cpy: {:}ms ({:}ns)",
			std::chrono::duration_cast<std::chrono::milliseconds>(cpyEnd - cpyStart).count(),
			std::chrono::duration_cast<std::chrono::nanoseconds>(cpyEnd - cpyStart).count() /
			writeAsset->payload.size()
		) << std::endl;

		delete writeAsset;
		delete readAsset;
	}

	TEST(LayoutArchive, OptVectorizedSliceSubType) {

		BufferArchive archive {};
		DataLayout<TestSerialSubTypeVectorizedSliceAsset> layout {};
		layout.describe();

		TypedLayoutArchive<TestSerialSubTypeVectorizedSliceAsset> arch { archive, &layout };

		auto writeAsset = new TestSerialSubTypeVectorizedSliceAsset();
		//writeAsset->payload.resize(200'000'000, { 1231uLL, 738956uLL, 2.32906F, 9230.35F });
		writeAsset->payload.resize(2'000'000, { 1231uLL, 738956uLL, 2.32906F, 9230.35F });
		archive.resize(
			// Linear payload size
			writeAsset->payload.size() * sizeof(decltype(TestSerialSubTypeVectorizedSliceAsset::payload)::value_type) +
			// Slice meta data size
			sizeof(u64) +
			//
			sizeof(u64) * 3uLL
		);

		const auto writeStart { std::chrono::high_resolution_clock::now() };
		arch << writeAsset;
		const auto writeEnd { std::chrono::high_resolution_clock::now() };

		archive.seek(0);

		auto readAsset = new TestSerialSubTypeVectorizedSliceAsset();

		const auto readStart { std::chrono::high_resolution_clock::now() };
		arch >> readAsset;
		const auto readEnd { std::chrono::high_resolution_clock::now() };

		EXPECT_EQ(writeAsset->payload.size(), readAsset->payload.size());
		EXPECT_EQ(writeAsset->payload.capacity(), readAsset->payload.capacity());

		/**/

		const auto memCpyStart { std::chrono::high_resolution_clock::now() };
		Vector<TestSubTypePayload> memCpyTarget {};
		memCpyTarget.resize(writeAsset->payload.size());
		std::memcpy(
			memCpyTarget.data(),
			writeAsset->payload.data(),
			writeAsset->payload.size() * sizeof(TestSubTypePayload)
		);
		const auto memCpyEnd { std::chrono::high_resolution_clock::now() };

		/**/

		memCpyTarget.clear();
		memCpyTarget.shrink_to_fit();

		/**/

		const auto cpyStart { std::chrono::high_resolution_clock::now() };
		memCpyTarget.reserve(writeAsset->payload.size());
		for (const auto& entry : writeAsset->payload) {
			memCpyTarget.emplace_back(entry);
		}
		const auto cpyEnd { std::chrono::high_resolution_clock::now() };

		/**/

		std::cout << "[Opt] " << std::format(
			"Serialization: {:}ms ({:}ns)",
			std::chrono::duration_cast<std::chrono::milliseconds>(writeEnd - writeStart).count(),
			std::chrono::duration_cast<std::chrono::nanoseconds>(writeEnd - writeStart).count() /
			writeAsset->payload.size()
		) << std::format(
			" | Deserialization: {:}ms ({:}ns)",
			std::chrono::duration_cast<std::chrono::milliseconds>(readEnd - readStart).count(),
			std::chrono::duration_cast<std::chrono::nanoseconds>(readEnd - readStart).count() /
			writeAsset->payload.size()
		) << std::format(
			" | MemCpy: {:}ms ({:}ns)",
			std::chrono::duration_cast<std::chrono::milliseconds>(memCpyEnd - memCpyStart).count(),
			std::chrono::duration_cast<std::chrono::nanoseconds>(memCpyEnd - memCpyStart).count() /
			writeAsset->payload.size()
		) << std::format(
			" | Cpy: {:}ms ({:}ns)",
			std::chrono::duration_cast<std::chrono::milliseconds>(cpyEnd - cpyStart).count(),
			std::chrono::duration_cast<std::chrono::nanoseconds>(cpyEnd - cpyStart).count() /
			writeAsset->payload.size()
		) << std::endl;

		delete writeAsset;
		delete readAsset;
	}
}
