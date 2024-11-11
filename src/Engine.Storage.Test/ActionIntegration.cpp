#include "pch.h"

/**/

#include "IntegrationFixture.hpp"

#include <Engine.Common/Guid.hpp>
#include <Engine.Config/Config.hpp>
#include <Engine.Resource.Archive/StorageReadonlyArchive.hpp>
#include <Engine.Resource.Archive/StorageReadWriteArchive.hpp>
#include <Engine.Resource.Blob/Blob.hpp>
#include <Engine.Resource.Blob/Memory/ByteSpanBlob.hpp>
#include <Engine.Storage.Action/Context.hpp>
#include <Engine.Storage.Action/Access/ExclusiveIoResource.hpp>
#include <Engine.Storage.Action/Action/StageTypeInfo.hpp>
#include <Engine.Storage.Registry/Options/MemoryStorageDescriptor.hpp>
#include <Engine.Storage.Registry/Options/MemoryStorageOption.hpp>
#include <Engine.Storage.Registry/Options/StorageDescriptor.hpp>
#include <Engine.Storage.Registry/Registry/StorageRegistry.hpp>
#include <Engine.Storage.Registry/Url/MemoryUrl.hpp>
#include <Engine.Storage.System/StorageSystem.hpp>
#include <Engine.Storage.System/Details/StorageActionResolver.hpp>

/**/

using namespace hg::engine::storage::system;
using namespace hg::engine::storage;
using namespace hg::engine::resource;
using namespace hg;

namespace StorageModule {
	class QueryIntegrationFixture :
		public testing::Test {
	public:
		~QueryIntegrationFixture() override = default;

	public:
		Context _context {};
		StorageRegistry _registry {};
		StorageActionResolver _resolver {};
		StorageSystem _system { _registry, _resolver };

	protected:
		void SetUp() override {
			engine::Config cfg {};
			_registry.setup(cfg);
		}

		void TearDown() override {
			_registry.tidy();
		}
	};
}

/**/

static void supportAddMemStoreBlobQueryStages(_Inout_ ref<StorageActionResolver> resolver_);

static void supportAddArchiveStoreQueryStages(_Inout_ ref<StorageActionResolver> resolver_);

static void supportAddMemStoreBlobMutationStages(_Inout_ ref<StorageActionResolver> resolver_);

static void supportAddArchiveStoreMutationStages(_Inout_ ref<StorageActionResolver> resolver_);

/**/

namespace StorageModule {
	TEST_F(QueryIntegrationFixture, SingleLayerQuery) {

		// Prepare stages for single layer query
		supportAddMemStoreBlobQueryStages(_resolver);

		/**/

		constexpr auto validation_sequence = StringView { "Hello World!"sv };
		auto memStorage = Arci<IStorage> {};

		/**/

		{
			auto guid = Guid {};
			GuidGenerate(guid);

			const auto memUrl = MemoryUrl { ::hg::move(guid) };
			constexpr auto requestSize = 32uLL;
			constexpr auto properties = MemoryProperty::eUndefined;

			/**/

			auto descriptor = MemoryStorageDescriptor { clone(memUrl), requestSize, properties };
			memStorage = _registry.insert(::hg::move(descriptor));
			ASSERT_NE(memStorage, nullptr);
		}

		/**/

		{
			auto& memory = clone(memStorage).into<MemoryStorage>()->_memory;
			std::copy_n(
				std::bit_cast<const ptr<const _::byte>>(validation_sequence.data()),
				validation_sequence.size(),
				memory.span().data()
			);
			memory.setAsTouched(validation_sequence.size());
		}

		/**/

		auto resolved = false;
		const auto result = _system.query(
			clone(memStorage),
			[resolved = &resolved, validation_sequence](_In_ cref<Blob> blob_) {

				auto holder = Array<_::byte, validation_sequence.size()> {};
				const auto loaded = blob_.read(streamoff {}, std::span { holder.data(), holder.size() });
				EXPECT_GE(validation_sequence.size(), loaded.size());

				auto fetched = StringView { std::bit_cast<const char*>(holder.data()), holder.size() };
				EXPECT_EQ(validation_sequence, fetched);

				*resolved = true;
				return fetched.size();
			}
		);

		EXPECT_TRUE(resolved);
		ASSERT_TRUE(result.has_value());
		EXPECT_EQ(result.value(), validation_sequence.size());
	}

	TEST_F(QueryIntegrationFixture, MultiLayerQuery) {

		// Prepare stages for multi layer query
		supportAddMemStoreBlobQueryStages(_resolver);
		supportAddArchiveStoreQueryStages(_resolver);

		/**/

		constexpr auto validation_sequence = StringView { "Hello World!"sv };
		auto memStorage = Arci<IStorage> {};
		auto archStorage = Arci<IStorage> {};

		/**/

		{
			auto guid = Guid {};
			GuidGenerate(guid);

			const auto memUrl = MemoryUrl { ::hg::move(guid) };
			constexpr auto requestSize = 32uLL;
			constexpr auto properties = MemoryProperty::eUndefined;

			/**/

			auto descriptor = MemoryStorageDescriptor { clone(memUrl), requestSize, properties };
			memStorage = _registry.insert(::hg::move(descriptor));
			ASSERT_NE(memStorage, nullptr);
		}

		/**/

		{
			auto& memory = clone(memStorage).into<MemoryStorage>()->_memory;
			std::copy_n(
				std::bit_cast<const ptr<const _::byte>>(validation_sequence.data()),
				validation_sequence.size(),
				memory.span().data()
			);
			memory.setAsTouched(validation_sequence.size());
		}

		/**/

		{
			auto guid = Guid {};
			GuidGenerate(guid);

			const auto archUrl = ArchiveUrl { ::hg::move(guid) };

			/**/

			auto descriptor = ArchiveStorageDescriptor { clone(archUrl), clone(memStorage) };
			archStorage = _registry.insert(::hg::move(descriptor));
			ASSERT_NE(archStorage, nullptr);
		}

		/**/

		auto resolved = false;
		const auto result = _system.query(
			clone(archStorage).into<ArchiveStorage>(),
			[resolved = &resolved, validation_sequence](_In_ ref<StorageReadonlyArchive> archive_) {

				auto holder = Array<u8, validation_sequence.size()> {};
				::hg::assertrt(archive_.totalSize() >= holder.size());

				archive_.seek(0);
				for (auto& c : holder) {
					archive_ >> c;
				}

				auto fetched = StringView { std::bit_cast<const char*>(holder.data()), holder.size() };
				EXPECT_EQ(validation_sequence, fetched);

				*resolved = true;
				return archive_.tell();
			}
		);

		EXPECT_TRUE(resolved);
		ASSERT_TRUE(result.has_value());
		EXPECT_EQ(result.value(), validation_sequence.size());
	}
}

/**/

namespace StorageModule {
	TEST_F(MutationIntegrationFixture, SingleLayerMutation) {

		// Prepare stages for single layer query
		supportAddMemStoreBlobMutationStages(_resolver);

		/**/

		constexpr auto validation_sequence = StringView { "Hello World!"sv };

		auto memStorage = Arci<IStorage> {};

		/**/

		{
			auto guid = Guid {};
			GuidGenerate(guid);

			const auto memUrl = MemoryUrl { ::hg::move(guid) };
			constexpr auto requestSize = 32uLL;
			constexpr auto properties = MemoryProperty::eUndefined;

			/**/

			auto descriptor = MemoryStorageDescriptor { clone(memUrl), requestSize, properties };
			memStorage = _registry.insert(::hg::move(descriptor));
			ASSERT_NE(memStorage, nullptr);
		}

		/**/

		auto resolved = false;
		const auto result = _system.mutate(
			clone(memStorage),
			[resolved = &resolved, validation_sequence](_In_ ref<Blob> blob_) {

				blob_.write(
					streamoff {},
					std::span { std::bit_cast<_::byte*>(validation_sequence.data()), validation_sequence.size() }
				);

				*resolved = true;
				return blob_.size();
			}
		);

		EXPECT_TRUE(resolved);
		ASSERT_TRUE(result.has_value());
		EXPECT_EQ(result.value(), validation_sequence.size());

		/**/

		{
			const auto& memory = clone(memStorage).into<MemoryStorage>()->_memory;
			EXPECT_GE(validation_sequence.size(), memory.touched());
			const auto written = StringView { std::bit_cast<const char*>(memory.touchedSpan().data()), validation_sequence.size() };
			EXPECT_EQ(validation_sequence, written);
		}
	}

	TEST_F(MutationIntegrationFixture, MultiLayerMutation) {

		// Prepare stages for multi layer query
		supportAddMemStoreBlobMutationStages(_resolver);
		supportAddArchiveStoreMutationStages(_resolver);

		/**/

		constexpr auto validation_sequence = StringView { "Hello World!"sv };
		auto memStorage = Arci<IStorage> {};
		auto archStorage = Arci<IStorage> {};

		/**/

		{
			auto guid = Guid {};
			GuidGenerate(guid);

			const auto memUrl = MemoryUrl { ::hg::move(guid) };
			constexpr auto requestSize = 32uLL;
			constexpr auto properties = MemoryProperty::eUndefined;

			/**/

			auto descriptor = MemoryStorageDescriptor { clone(memUrl), requestSize, properties };
			memStorage = _registry.insert(::hg::move(descriptor));
			ASSERT_NE(memStorage, nullptr);
		}

		/**/

		{
			auto guid = Guid {};
			GuidGenerate(guid);

			const auto archUrl = ArchiveUrl { ::hg::move(guid) };

			/**/

			auto descriptor = ArchiveStorageDescriptor { clone(archUrl), clone(memStorage) };
			archStorage = _registry.insert(::hg::move(descriptor));
			ASSERT_NE(archStorage, nullptr);
		}

		/**/

		auto resolved = false;
		const auto result = _system.mutate(
			clone(archStorage).into<ArchiveStorage>(),
			[resolved = &resolved, validation_sequence](_In_ auto& archive_) {

				archive_.seek(0);
				for (const auto& c : validation_sequence) {
					archive_ << c;
				}

				*resolved = true;
				return archive_.tell();
			}
		);

		EXPECT_TRUE(resolved);
		ASSERT_TRUE(result.has_value());
		EXPECT_EQ(result.value(), validation_sequence.size());

		/**/

		{
			auto memory = clone(memStorage).into<MemoryStorage>()->_memory.span();
			const auto written = StringView { std::bit_cast<const char*>(memory.data()), validation_sequence.size() };
			EXPECT_EQ(validation_sequence, written);
		}
	}
}

/**/

#pragma region Support Functions

void supportAddMemStoreBlobQueryStages(ref<StorageActionResolver> resolver_) {

	decltype(DynamicStage::acqFn) acqFn = [](
		Context& ctx_,
		cref<Arci<IStorage>> storage_,
		[[maybe_unused]] const auto& input_
	) -> Result<meta::WithType<Rc<IoResourceAccessorBase>>, Action::acq_error_type> {

		auto memStore = storage_.into<MemoryStorage>();
		if (not IsType<MemoryStorage>(*memStore)) {
			return Unexpected<Query::acq_error_type> {
				IllegalTypeError { "Invalid storage type at dynamic query stage." }
			};
		}

		/**/

		using resolve_type = std::function<ref<ByteSpanBlob>(ResourceAccessMode)>;
		using resource_type = decltype(ExclusiveIoResource { std::declval<resolve_type>() });

		/**/

		const auto& memory = memStore->_memory;

		auto value = Rc<ByteSpanBlob>::create(memory.touchedSpan());
		auto cachedValue = ctx_.caches.add(clone(storage_), ::hg::move(value));

		/**/

		auto resolver = [_obj = ::hg::move(cachedValue)](ResourceAccessMode) mutable -> ref<ByteSpanBlob> {
			return *_obj;
		};

		/**/

		auto resource = Rc<resource_type>::create(::hg::move(resolver));
		auto storedResource = ctx_.store.add(::hg::move(memStore).into<IStorage>(), ::hg::move(resource));

		/**/

		return Expected {
			meta::WithType<Rc<IoResourceAccessorBase>>::from(
				Rc<IoResourceAccessor<const ByteSpanBlob>>::create(storedResource->acquireReadonly())
			)
		};
	};

	decltype(DynamicStage::relFn) relFn = [](Context& ctx_, cref<Arci<IStorage>>, auto&& val_) -> void {
		::hg::forward<decltype(val_)>(val_).value.reset();

		ctx_.store.map.clear();
		ctx_.caches.map.clear();
	};

	auto stageTypeInfo = StageTypeInfo {
		.storage = refl::PartialTypeInfo::from<MemoryStorage>(),
		.resource = refl::FullTypeInfo::from<const ByteSpanBlob>(),
		.input = { nullptr }
	};

	resolver_.useStage<DynamicStage>(::hg::move(stageTypeInfo), ::hg::move(acqFn), ::hg::move(relFn));
}

void supportAddArchiveStoreQueryStages(ref<StorageActionResolver> resolver_) {

	decltype(DynamicStage::acqFn) acqFn = [](
		Context& ctx_,
		cref<Arci<IStorage>> storage_,
		const auto& input_
	) -> Result<meta::WithType<Rc<IoResourceAccessorBase>>, Action::acq_error_type> {

		auto archStorage = storage_.into<ArchiveStorage>();
		if (not IsType<ArchiveStorage>(*archStorage)) {
			return Unexpected<Action::acq_error_type> {
				IllegalTypeError { "Invalid storage type at dynamic query stage." }
			};
		}

		const auto& input = reinterpret_cast<const meta::WithType<Rc<IoResourceAccessor<const ByteSpanBlob>>>&>(input_);
		if (input.value == nullptr || not input.valid()) {
			return Unexpected<Action::acq_error_type> {
				IllegalTypeError { "Invalid input type at dynamic query stage." }
			};
		}

		/**/

		using resolve_mut_type = std::function<ref<StorageReadWriteArchive>(ResourceAccessMode)>;
		using resolve_const_type = std::function<ref<StorageReadonlyArchive>(ResourceAccessMode)>;
		using resource_type = decltype(ExclusiveIoResource { std::declval<resolve_mut_type>(), std::declval<resolve_const_type>() });

		/**/

		auto value = Rc<StorageReadonlyArchive>::create(input.value->get(), streamoff {}, input.value->get().size());
		auto cachedValue = ctx_.caches.add(clone(storage_), ::hg::move(value));

		/**/

		auto resolveMut = [](ResourceAccessMode) -> ref<StorageReadWriteArchive> {
			::hg::panic();
		};
		auto resolveConst = [_obj = ::hg::move(cachedValue)](ResourceAccessMode) mutable -> ref<StorageReadonlyArchive> {
			return *_obj;
		};

		/**/

		auto resource = Rc<resource_type>::create(::hg::move(resolveMut), ::hg::move(resolveConst));
		auto storedResource = ctx_.store.add(clone(storage_).into<IStorage>(), ::hg::move(resource));

		/**/

		return Expected {
			meta::WithType<Rc<IoResourceAccessorBase>>::from(
				Rc<IoResourceAccessor<StorageReadonlyArchive>>::create(storedResource->acquireReadonly())
			)
		};
	};

	decltype(DynamicStage::relFn) relFn = [](Context& ctx_, cref<Arci<IStorage>>, auto&& val_) -> void {
		::hg::forward<decltype(val_)>(val_).value.reset();

		// Problem: We need to identify resources and cached access to invalidate them.
		// Warning: ctx_.store.map.clear();
		// Warning: ctx_.caches.map.clear();
	};

	auto stageTypeInfo = StageTypeInfo {
		.storage = refl::PartialTypeInfo::from<ArchiveStorage>(),
		.resource = refl::FullTypeInfo::from<StorageReadonlyArchive>(),
		.input = refl::FullTypeInfo::from<const ByteSpanBlob>()
	};

	resolver_.useStage<DynamicStage>(::hg::move(stageTypeInfo), ::hg::move(acqFn), ::hg::move(relFn));
}

void supportAddMemStoreBlobMutationStages(ref<StorageActionResolver> resolver_) {

	decltype(DynamicStage::acqFn) acqFn = [](
		Context& ctx_,
		cref<Arci<IStorage>> storage_,
		[[maybe_unused]] const auto& input_
	) -> Result<meta::WithType<Rc<IoResourceAccessorBase>>, Action::acq_error_type> {

		auto memStore = storage_.into<MemoryStorage>();
		if (not IsType<MemoryStorage>(*memStore)) {
			return Unexpected<Query::acq_error_type> {
				IllegalTypeError { "Invalid storage type at dynamic query stage." }
			};
		}

		/**/

		using resolve_type = std::function<ref<ByteSpanBlob>(ResourceAccessMode)>;
		using resource_type = decltype(ExclusiveIoResource { std::declval<resolve_type>() });

		/**/

		auto value = Rc<ByteSpanBlob>::create(memStore->_memory.span());
		auto cachedValue = ctx_.caches.add(clone(storage_), ::hg::move(value));

		/**/

		auto resolver = [_obj = ::hg::move(cachedValue)](ResourceAccessMode) mutable -> ref<ByteSpanBlob> {
			return *_obj;
		};

		/**/

		auto resource = Rc<resource_type>::create(::hg::move(resolver));
		auto storedResource = ctx_.store.add(::hg::move(memStore).into<IStorage>(), ::hg::move(resource));

		/**/

		return Expected {
			meta::WithType<Rc<IoResourceAccessorBase>>::from(
				Rc<IoResourceAccessor<ByteSpanBlob>>::create(storedResource->acquireReadWrite())
			)
		};
	};

	decltype(DynamicStage::relFn) relFn = [](Context& ctx_, cref<Arci<IStorage>>, auto&& val_) -> void {
		::hg::forward<decltype(val_)>(val_).value.reset();

		ctx_.store.map.clear();
		ctx_.caches.map.clear();
	};

	auto stageTypeInfo = StageTypeInfo {
		.storage = refl::PartialTypeInfo::from<MemoryStorage>(),
		.resource = refl::FullTypeInfo::from<ByteSpanBlob>(),
		.input = { nullptr }
	};

	resolver_.useStage<DynamicStage>(::hg::move(stageTypeInfo), ::hg::move(acqFn), ::hg::move(relFn));
}

void supportAddArchiveStoreMutationStages(ref<StorageActionResolver> resolver_) {

	decltype(DynamicStage::acqFn) acqFn = [](
		Context& ctx_,
		cref<Arci<IStorage>> storage_,
		const auto& input_
	) -> Result<meta::WithType<Rc<IoResourceAccessorBase>>, Action::acq_error_type> {

		auto archStorage = storage_.into<ArchiveStorage>();
		if (not IsType<ArchiveStorage>(*archStorage)) {
			return Unexpected<Action::acq_error_type> {
				IllegalTypeError { "Invalid storage type at dynamic query stage." }
			};
		}

		const auto& input = reinterpret_cast<const meta::WithType<Rc<IoResourceAccessor<ByteSpanBlob>>>&>(input_);
		if (input.value == nullptr || not input.valid()) {
			return Unexpected<Action::acq_error_type> {
				IllegalTypeError { "Invalid input type at dynamic query stage." }
			};
		}

		/**/

		using resolve_mut_type = std::function<ref<StorageReadWriteArchive>(ResourceAccessMode)>;
		using resolve_const_type = std::function<ref<StorageReadonlyArchive>(ResourceAccessMode)>;
		using resource_type = decltype(ExclusiveIoResource { std::declval<resolve_mut_type>(), std::declval<resolve_const_type>() });

		/**/

		auto value = Rc<StorageReadWriteArchive>::create(input.value->get(), streamoff {}, input.value->get().size());
		auto cachedValue = ctx_.caches.add(clone(storage_), ::hg::move(value));

		/**/

		auto resolveMut = [_obj = ::hg::move(cachedValue)](ResourceAccessMode) mutable -> ref<engine::resource::StorageReadWriteArchive> {
			return *_obj;
		};
		auto resolveConst = [](ResourceAccessMode) -> ref<engine::resource::StorageReadonlyArchive> {
			::hg::panic();
		};

		/**/

		auto resource = Rc<resource_type>::create(::hg::move(resolveMut), ::hg::move(resolveConst));
		auto storedResource = ctx_.store.add(clone(storage_).into<IStorage>(), ::hg::move(resource));

		/**/

		return Expected {
			meta::WithType<Rc<IoResourceAccessorBase>>::from(
				Rc<IoResourceAccessor<StorageReadWriteArchive>>::create(storedResource->acquireReadWrite())
			)
		};
	};

	decltype(DynamicStage::relFn) relFn = [](Context& ctx_, cref<Arci<IStorage>>, auto&& val_) -> void {
		::hg::forward<decltype(val_)>(val_).value.reset();

		// Problem: We need to identify resources and cached access to invalidate them.
		// Warning: ctx_.store.map.clear();
		// Warning: ctx_.caches.map.clear();
	};

	auto stageTypeInfo = StageTypeInfo {
		.storage = refl::PartialTypeInfo::from<ArchiveStorage>(),
		.resource = refl::FullTypeInfo::from<StorageReadWriteArchive>(),
		.input = refl::FullTypeInfo::from<ByteSpanBlob>()
	};

	resolver_.useStage<DynamicStage>(::hg::move(stageTypeInfo), ::hg::move(acqFn), ::hg::move(relFn));
}

#pragma endregion
