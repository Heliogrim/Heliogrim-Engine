#include "pch.h"

/**/

#include <Engine.Reflect/IsType.hpp>
#include <Engine.Resource.Archive/StorageReadWriteArchive.hpp>
#include <Engine.Resource.Blob/File/LfsSyncBlob.hpp>
#include <Engine.Resource.Blob/Memory/ByteSpanBlob.hpp>
#include <Engine.Resource.Package/Package.hpp>
#include <Engine.Storage.Action/Access/ExclusiveIoResource.hpp>
#include <Engine.Storage.Action/Action/MutationStaticGen.hpp>
#include <Engine.Storage.Boot/Action/ActionResolverInit.hpp>
#include <Engine.Storage.Registry/Registry/StorageRegistry.hpp>
#include <Engine.Storage.System/StorageSystem.hpp>
#include <Engine.Storage.System/Details/StorageActionResolver.hpp>

using namespace hg::engine::storage::system;
using namespace hg::engine::storage;
using namespace hg::engine::resource;
using namespace hg;

namespace StorageModule {
	template <typename ResultType_>
	using stage_result_type = Result<IoResourceAccessor<ResultType_>, AccessError>;

	class MutationTypeResolutionFixture :
		public testing::Test {
	public:
		Context _context {};
		StorageRegistry _registry {};
		StorageActionResolver _resolver {};
		StorageSystem _system { _registry, _resolver };

	public:
		void SetUp() override {
			// __noop();
		}

		void TearDown() override {
			// __noop();
		}
	};
}

namespace StorageModule {
	TEST_F(MutationTypeResolutionFixture, FetchStrictStatic) {

		// TODO: Upsert `LocalFileStorage` + `null` -(static)-> ` LfsSyncBlob`
		// TODO: Upsert `PackageStorage` + ` LfsSyncBlob` -(static)-> ` Package`
		// TODO: Upsert `ArchiveStorage` + ` Package` -(static)-> `StorageReadWriteArchive`

		// TODO: Request `ArchiveStorage` -> `StorageReadWriteArchive`

		using namespace ::hg::engine::storage;

		/**/

		initActionResolver(_system, _resolver);

		/**/

		auto base = Arci<system::LocalFileStorage>::create(fs::Path {}, true, true, true, true);
		auto inter = Arci<system::PackageStorage>::create(clone(base).into<IStorage>(), true, true, true, true);
		auto dist = Arci<system::ArchiveStorage>::create(clone(inter).into<IStorage>(), true, true, true, true);

		/**/

		auto signature = ActionTypeInfo {
			.targetResourceInfo = refl::FullTypeInfo::from<Archive>(),
			.stageTypeInfos = {}
		};

		signature.stageTypeInfos.emplace_back(StageTypeInfo { refl::PartialTypeInfo { dist->getMetaClass() } });

		auto next = clone(dist->getBacking());
		while (next != nullptr) {
			signature.stageTypeInfos.emplace_back(StageTypeInfo { refl::PartialTypeInfo { next->getMetaClass() } });

			if (IsType<system::PackageStorage>(*next)) {
				next = static_cast<cref<system::PackageStorage>>(*next).getBacking();

			} else if (IsType<system::ArchiveStorage>(*next)) {
				next = static_cast<cref<system::ArchiveStorage>>(*next).getBacking();

			} else {
				next = nullptr;
			}
		}

		/**/

		//const auto result = system.mutate(
		//	clone(dist),
		//	[](ref<engine::resource::StorageReadWriteArchive> archive_) {
		//		archive_.flush();
		//	}
		//);
		const auto factory = _resolver.resolveMutation(signature);
		EXPECT_TRUE(factory.has_value());

		auto mutation = (*factory)();

		const auto preResult = mutation->prepare(clone(dist).into<IStorage>());
		//const auto acqResult = mutation->acquire(context);
		//mutation->exec([](const auto&) -> void {});
		//mutation->release(context);
		mutation->cleanup();
	}

	TEST_F(MutationTypeResolutionFixture, FetchCovariantStatic) {
		// Note: Do not upsert any static stages or actions

		// Upsert `LocalFileStorage` + `null` -> `LfsSyncBlob`
		using stage0_type = decltype(StaticStage {
			.storage = Arci<system::LocalFileStorage> {},
			.acqFn = [](Context&, cref<Arci<system::LocalFileStorage>>) -> stage_result_type<LfsSyncBlob> {
				return Unexpected { AccessError { "Intentional Failure." } };
			},
			.access = IoResourceAccessor<LfsSyncBlob> {},
			.relFn = [](Context&, cref<Arci<LocalFileStorage>>, IoResourceAccessor<LfsSyncBlob>&& val_) -> void {
				::hg::forward<decltype(val_)>(val_).reset();
			}
		});
		using mutation_type = MutationStaticGen<stage0_type>;

		auto actionTypeInfo = ActionTypeInfo {
			.targetResourceInfo = refl::FullTypeInfo::from<LfsSyncBlob>(),
			.stageTypeInfos = {
				StageTypeInfo {
					.storage = refl::PartialTypeInfo::from<system::LocalFileStorage>(),
					.resource = refl::FullTypeInfo::from<LfsSyncBlob>(),
					.input = { nullptr }
				}
			}
		};

		_resolver.use<mutation_type>(::hg::move(actionTypeInfo));

		// Request `LocalFileStorage` -> `Blob`
		auto signature = ActionTypeInfo {
			.targetResourceInfo = refl::FullTypeInfo::from<Blob>(),
			.stageTypeInfos = {
				StageTypeInfo {
					refl::PartialTypeInfo::from<system::LocalFileStorage>()
				}
			}
		};

		const auto factory = _resolver.resolveMutation(signature);
		ASSERT_TRUE(factory.has_value());

		/**/

		auto dummy = Arci<system::LocalFileStorage>::create(fs::Path {}, true, true, true, true);

		auto mutation = (*factory)();

		const auto preResult = mutation->prepare(clone(dummy).into<IStorage>());
		EXPECT_TRUE(preResult.has_value());
		// Note: Intermediate stages are discarded for type resolution tests...
		mutation->cleanup();
	}

	TEST_F(MutationTypeResolutionFixture, FetchStrictDynamic) {
		// Note: Do not upsert any static stages or actions

		// Upsert `LocalFileStorage` + `null` -(dyn)-> `LfsSyncBlob`
		{
			decltype(DynamicStage::acqFn) acqFn = [](Context&, cref<Arci<IStorage>>, const auto&) -> auto {
				return Unexpected<Mutation::acq_error_type> { AccessError { "Intentional Failure." } };
			};
			decltype(DynamicStage::relFn) relFn = [](Context&, cref<Arci<IStorage>>, auto&& val_) -> void {
				::hg::forward<decltype(val_)>(val_).value.reset();
			};

			auto stageTypeInfo = StageTypeInfo {
				.storage = refl::PartialTypeInfo::from<system::LocalFileStorage>(),
				.resource = refl::FullTypeInfo::from<LfsSyncBlob>(),
				.input = { nullptr }
			};

			_resolver.useStage<DynamicStage>(::hg::move(stageTypeInfo), ::hg::move(acqFn), ::hg::move(relFn));
		}

		// Upsert `PackageStorage` + `LfsSyncBlob` -(dyn)-> `Package`
		{
			decltype(DynamicStage::acqFn) acqFn = [](Context&, cref<Arci<IStorage>>, const auto&) -> auto {
				return Unexpected<Mutation::acq_error_type> { AccessError { "Intentional Failure." } };
			};
			decltype(DynamicStage::relFn) relFn = [](Context&, cref<Arci<IStorage>>, auto&& val_) -> void {
				::hg::forward<decltype(val_)>(val_).value.reset();
			};

			auto stageTypeInfo = StageTypeInfo {
				.storage = refl::PartialTypeInfo::from<system::PackageStorage>(),
				.resource = refl::FullTypeInfo::from<Package>(),
				.input = refl::FullTypeInfo::from<LfsSyncBlob>()
			};

			_resolver.useStage<DynamicStage>(::hg::move(stageTypeInfo), ::hg::move(acqFn), ::hg::move(relFn));
		}

		// Upsert `ArchiveStorage` + `Package` -(dyn)-> `StorageReadWriteArchive`
		{
			decltype(DynamicStage::acqFn) acqFn = [](Context&, cref<Arci<IStorage>>, const auto&) -> auto {
				return Unexpected<Mutation::acq_error_type> { AccessError { "Intentional Failure." } };
			};
			decltype(DynamicStage::relFn) relFn = [](Context&, cref<Arci<IStorage>>, auto&& val_) -> void {
				::hg::forward<decltype(val_)>(val_).value.reset();
			};

			auto stageTypeInfo = StageTypeInfo {
				.storage = refl::PartialTypeInfo::from<system::ArchiveStorage>(),
				.resource = refl::FullTypeInfo::from<StorageReadWriteArchive>(),
				.input = refl::FullTypeInfo::from<Package>()
			};

			_resolver.useStage<DynamicStage>(::hg::move(stageTypeInfo), ::hg::move(acqFn), ::hg::move(relFn));
		}

		// Request `ArchiveStorage` -> `StorageReadWriteArchive`
		auto signature = ActionTypeInfo {
			.targetResourceInfo = refl::FullTypeInfo::from<StorageReadWriteArchive>(),
			.stageTypeInfos = {
				StageTypeInfo {
					refl::PartialTypeInfo::from<system::ArchiveStorage>()
				},
				StageTypeInfo {
					refl::PartialTypeInfo::from<system::PackageStorage>()
				},
				StageTypeInfo {
					refl::PartialTypeInfo::from<system::LocalFileStorage>()
				}
			}
		};

		const auto factory = _resolver.resolveMutation(signature);
		ASSERT_TRUE(factory.has_value());

		/**/

		auto lfsDummy = Arci<system::LocalFileStorage>::create(fs::Path {}, true, true, true, true);
		auto packageDummy = Arci<system::PackageStorage>::create(clone(lfsDummy).into<IStorage>(), true, true, true, true);
		auto archiveDummy = Arci<system::ArchiveStorage>::create(clone(packageDummy).into<IStorage>(), true, true, true, true);

		auto mutation = (*factory)();
		ASSERT_TRUE(mutation);

		const auto preResult = mutation->prepare(clone(archiveDummy).into<IStorage>());
		EXPECT_TRUE(preResult.has_value());
		// Note: Intermediate stages are discarded for type resolution tests...
		mutation->cleanup();
	}

	TEST_F(MutationTypeResolutionFixture, FetchCovariantDynamic) {
		// Note: Do not upsert any static stages or actions

		// Upsert `LocalFileStorage` + `null` -(dyn)-> `LfsSyncBlob`
		decltype(DynamicStage::acqFn) acqFn = [](Context&, cref<Arci<IStorage>>, const auto&) -> auto {
			return Unexpected<Mutation::acq_error_type> { AccessError { "Intentional Failure." } };
		};
		decltype(DynamicStage::relFn) relFn = [](Context&, cref<Arci<IStorage>>, auto&& val_) -> void {
			::hg::forward<decltype(val_)>(val_).value.reset();
		};

		auto stageTypeInfo = StageTypeInfo {
			.storage = refl::PartialTypeInfo::from<system::LocalFileStorage>(),
			.resource = refl::FullTypeInfo::from<LfsSyncBlob>(),
			.input = { nullptr }
		};

		_resolver.useStage<DynamicStage>(::hg::move(stageTypeInfo), ::hg::move(acqFn), ::hg::move(relFn));

		// Request `LocalFileStorage` -> `Blob`
		auto signature = ActionTypeInfo {
			.targetResourceInfo = refl::FullTypeInfo::from<Blob>(),
			.stageTypeInfos = {
				StageTypeInfo {
					refl::PartialTypeInfo::from<system::LocalFileStorage>()
				}
			}
		};

		const auto factory = _resolver.resolveMutation(signature);
		ASSERT_TRUE(factory.has_value());

		/**/

		auto dummy = Arci<system::LocalFileStorage>::create(fs::Path {}, true, true, true, true);

		auto mutation = (*factory)();
		ASSERT_TRUE(mutation);

		const auto preResult = mutation->prepare(clone(dummy).into<IStorage>());
		EXPECT_TRUE(preResult.has_value());
		// Note: Intermediate stages are discarded for type resolution tests...
		mutation->cleanup();
	}

	TEST_F(MutationTypeResolutionFixture, FetchMorphedDynamic) {
		// Note: Do not upsert any static stages or actions

		// Upsert `LocalFileStorage` + `null` -(static)-> `LfsSyncBlob`
		using static_stage_type = decltype(StaticStage {
			.storage = Arci<system::LocalFileStorage> {},
			.acqFn = [](Context&, cref<Arci<system::LocalFileStorage>>) -> stage_result_type<LfsSyncBlob> {
				return Unexpected { AccessError { "Intentional Failure." } };
			},
			.access = IoResourceAccessor<LfsSyncBlob> {},
			.relFn = [](Context&, cref<Arci<LocalFileStorage>>, IoResourceAccessor<LfsSyncBlob>&& val_) -> void {
				::hg::forward<decltype(val_)>(val_).reset();
			}
		});

		auto stageTypeInfo = StageTypeInfo {
			.storage = refl::PartialTypeInfo::from<system::LocalFileStorage>(),
			.resource = refl::FullTypeInfo::from<LfsSyncBlob>(),
			.input = { nullptr }
		};

		_resolver.useStage<static_stage_type>(::hg::move(stageTypeInfo));

		// Upsert `ArchiveStorage` + `LfsSyncBlob` -(dyn)-> `StorageReadWriteArchive` // Note: Required to force morphing
		{
			decltype(DynamicStage::acqFn) acqFn = [](Context&, cref<Arci<IStorage>>, const auto&) -> auto {
				return Unexpected<Mutation::acq_error_type> { AccessError { "Intentional Failure." } };
			};
			decltype(DynamicStage::relFn) relFn = [](Context&, cref<Arci<IStorage>>, auto&& val_) -> void {
				::hg::forward<decltype(val_)>(val_).value.reset();
			};

			auto stageTypeInfo = StageTypeInfo {
				.storage = refl::PartialTypeInfo::from<system::ArchiveStorage>(),
				.resource = refl::FullTypeInfo::from<StorageReadWriteArchive>(),
				.input = refl::FullTypeInfo::from<LfsSyncBlob>()
			};

			_resolver.useStage<DynamicStage>(::hg::move(stageTypeInfo), ::hg::move(acqFn), ::hg::move(relFn));
		}

		// Request `ArchiveStorage` -> `StorageReadWriteArchive`
		auto signature = ActionTypeInfo {
			.targetResourceInfo = refl::FullTypeInfo::from<StorageReadWriteArchive>(),
			.stageTypeInfos = {
				StageTypeInfo {
					refl::PartialTypeInfo::from<system::ArchiveStorage>()
				},
				StageTypeInfo {
					refl::PartialTypeInfo::from<system::LocalFileStorage>()
				}
			}
		};

		const auto factory = _resolver.resolveMutation(signature);
		ASSERT_TRUE(factory.has_value());

		/**/

		auto lfsDummy = Arci<system::LocalFileStorage>::create(fs::Path {}, true, true, true, true);
		auto archiveDummy = Arci<system::ArchiveStorage>::create(clone(lfsDummy).into<IStorage>(), true, true, true, true);

		auto mutation = (*factory)();
		ASSERT_TRUE(mutation);

		const auto preResult = mutation->prepare(clone(archiveDummy).into<IStorage>());
		EXPECT_TRUE(preResult.has_value());
		// Note: Intermediate stages are discarded for type resolution tests...
		mutation->cleanup();
	}

	TEST_F(MutationTypeResolutionFixture, FetchCovariantDynamicComposed) {
		// Note: Do not upsert any static stages or actions

		// Upsert `MemoryStorage` + `null` -(dyn)-> `Blob`
		{
			decltype(DynamicStage::acqFn) acqFn = [](
				Context& ctx_,
				cref<Arci<IStorage>> storage_,
				const auto& input_
			) -> Result<meta::WithType<Rc<IoResourceAccessorBase>>, Mutation::acq_error_type> {

				auto memStore = storage_.into<system::MemoryStorage>();
				if (not IsType<system::MemoryStorage>(*memStore)) {
					return Unexpected<Mutation::acq_error_type> { IllegalTypeError { "Invalid storage type at dynamic mutation stage." } };
				}

				if (input_.value != nullptr) {
					return Unexpected<Mutation::acq_error_type> {
						IllegalArgumentError { "Illegally provided an input resource to a primary stage transformer." }
					};
				}

				/**/

				using resolve_type = std::function<ref<engine::resource::ByteSpanBlob>(ResourceAccessMode)>;
				using resource_type = decltype(ExclusiveIoResource { std::declval<resolve_type>() });

				/**/

				auto value = Rc<engine::resource::ByteSpanBlob>::create(memStore->_memory.span());
				auto cachedValue = ctx_.caches.add(clone(storage_), ::hg::move(value));

				/**/

				auto resolver = [_obj = ::hg::move(cachedValue)](ResourceAccessMode) mutable -> ref<engine::resource::ByteSpanBlob> {
					return *_obj;
				};

				/**/

				auto resource = Rc<resource_type>::create(::hg::move(resolver));
				auto storedResource = ctx_.store.add(::hg::move(memStore).into<IStorage>(), ::hg::move(resource));

				/**/

				return Expected(
					meta::WithType<Rc<IoResourceAccessorBase>>::from(
						Rc<IoResourceAccessor<ByteSpanBlob>>::create(storedResource->acquireReadWrite())
					)
				);
			};
			decltype(DynamicStage::relFn) relFn = [](Context&, cref<Arci<IStorage>>, auto&& val_) -> void {
				::hg::forward<decltype(val_)>(val_).value.reset();
			};

			auto stageTypeInfo = StageTypeInfo {
				.storage = refl::PartialTypeInfo::from<system::MemoryStorage>(),
				.resource = refl::FullTypeInfo::from<Blob>(),
				.input = { nullptr }
			};

			_resolver.useStage<DynamicStage>(::hg::move(stageTypeInfo), ::hg::move(acqFn), ::hg::move(relFn));
		}

		// Upsert `ArchiveStorage` + `LfsSyncBlob` -(dyn)-> `StorageReadWriteArchive`
		{
			decltype(DynamicStage::acqFn) acqFn = [](Context&, cref<Arci<IStorage>>, const auto& input_) -> auto {
				const auto& maybeCasted = reinterpret_cast<cref<meta::WithType<Rc<ResourceAccessor<LfsSyncBlob>>>>>(input_);
				if (not maybeCasted.valid()) {
					return Unexpected<Mutation::acq_error_type> { IllegalTypeError { "Expected type Failure." } };
				}
				return Unexpected<Mutation::acq_error_type> { AccessError { "Unintentional Failure." } };
			};
			decltype(DynamicStage::relFn) relFn = [](Context&, cref<Arci<IStorage>>, auto&& val_) -> void {
				::hg::forward<decltype(val_)>(val_).value.reset();
			};

			auto stageTypeInfo = StageTypeInfo {
				.storage = refl::PartialTypeInfo::from<system::ArchiveStorage>(),
				.resource = refl::FullTypeInfo::from<StorageReadWriteArchive>(),
				.input = refl::FullTypeInfo::from<LfsSyncBlob>()
			};

			_resolver.useStage<DynamicStage>(::hg::move(stageTypeInfo), ::hg::move(acqFn), ::hg::move(relFn));
		}

		// Upsert `ArchiveStorage` + `const MemBlob` -(dyn)-> `StorageReadWriteArchive`
		{
			decltype(DynamicStage::acqFn) acqFn = [](Context&, cref<Arci<IStorage>>, const auto& input_) -> auto {
				const auto& maybeCasted = reinterpret_cast<cref<meta::WithType<Rc<IoResourceAccessor<ByteSpanBlob>>>>>(input_);
				if (not maybeCasted.valid()) {
					return Unexpected<Mutation::acq_error_type> { IllegalTypeError { "Unintentional Failure." } };
				}
				return Unexpected<Mutation::acq_error_type> {
					AccessError { "Expected preserving result to exist control-flow for this test." }
				};
			};
			decltype(DynamicStage::relFn) relFn = [](Context&, cref<Arci<IStorage>>, auto&& val_) -> void {
				::hg::forward<decltype(val_)>(val_).value.reset();
			};

			auto stageTypeInfo = StageTypeInfo {
				.storage = refl::PartialTypeInfo::from<system::ArchiveStorage>(),
				.resource = refl::FullTypeInfo::from<StorageReadWriteArchive>(),
				.input = refl::FullTypeInfo::from<ByteSpanBlob>()
			};

			_resolver.useStage<DynamicStage>(::hg::move(stageTypeInfo), ::hg::move(acqFn), ::hg::move(relFn));
		}

		// Request `ArchiveStorage` -> `StorageReadWriteArchive`
		auto signature = ActionTypeInfo {
			.targetResourceInfo = refl::FullTypeInfo::from<StorageReadWriteArchive>(),
			.stageTypeInfos = {
				StageTypeInfo {
					refl::PartialTypeInfo::from<system::ArchiveStorage>()
				},
				StageTypeInfo {
					refl::PartialTypeInfo::from<system::MemoryStorage>()
				}
			}
		};

		const auto factory = _resolver.resolveMutation(signature);
		ASSERT_TRUE(factory.has_value());

		/**/

		auto memDummy = Arci<system::MemoryStorage>::create(system::MemoryStorage::MemoryObject {}, true, true, true, true);
		auto archiveDummy = Arci<system::ArchiveStorage>::create(clone(memDummy).into<IStorage>(), true, true, true, true);

		auto mutation = (*factory)();
		ASSERT_TRUE(mutation);

		// Note: First internal type probe (`LfsSyncBlob`) should fail and we escape with secondary path
		const auto preResult = mutation->prepare(clone(archiveDummy).into<IStorage>());
		ASSERT_TRUE(preResult.has_value());

		const auto acqResult = mutation->acquire(_context);
		ASSERT_FALSE(acqResult.has_value());

		ASSERT_TRUE(acqResult.error().is<AccessError>());
		EXPECT_TRUE(StringView {acqResult.error().as<AccessError>().what()}.starts_with("Expected preserving result"sv));

		// Note: Intermediate stages are discarded for type resolution tests...
		mutation->release(_context);
		mutation->cleanup();
	}

	TEST_F(MutationTypeResolutionFixture, FetchMorphedCovariantDynamicComposed) {
		// Note: Do not upsert any static stages or actions

		// Upsert `MemoryStorage` + `null` -(dyn)-> `Blob`
		{
			decltype(DynamicStage::acqFn) acqFn = [](
				Context& ctx_,
				cref<Arci<IStorage>> storage_,
				const auto& input_
			) -> Result<meta::WithType<Rc<IoResourceAccessorBase>>, Mutation::acq_error_type> {

				auto memStore = storage_.into<system::MemoryStorage>();
				if (not IsType<system::MemoryStorage>(*memStore)) {
					return Unexpected<Mutation::acq_error_type> {
						IllegalTypeError { "Invalid storage type at dynamic mutation stage." }
					};
				}

				if (input_.value != nullptr) {
					return Unexpected<Mutation::acq_error_type> {
						IllegalArgumentError { "Illegally provided an input resource to a primary stage transformer." }
					};
				}

				/**/

				using resolve_type = std::function<ref<engine::resource::ByteSpanBlob>(ResourceAccessMode)>;
				using resource_type = decltype(ExclusiveIoResource { std::declval<resolve_type>() });

				/**/

				auto value = Rc<engine::resource::ByteSpanBlob>::create(memStore->_memory.span());
				auto cachedValue = ctx_.caches.add(clone(storage_), ::hg::move(value));

				/**/

				auto resolver = [_obj = ::hg::move(cachedValue)](ResourceAccessMode) mutable -> ref<engine::resource::ByteSpanBlob> {
					return *_obj;
				};

				/**/

				auto resource = Rc<resource_type>::create(::hg::move(resolver));
				auto storedResource = ctx_.store.add(::hg::move(memStore).into<IStorage>(), ::hg::move(resource));

				/**/

				return Expected(
					meta::WithType<Rc<IoResourceAccessorBase>>::from(
						Rc<IoResourceAccessor<ByteSpanBlob>>::create(storedResource->acquireReadWrite())
					)
				);
			};
			decltype(DynamicStage::relFn) relFn = [](Context& ctx_, cref<Arci<IStorage>>, auto&& val_) -> void {
				::hg::forward<decltype(val_)>(val_).value.reset();

				ctx_.store.map.clear();
				ctx_.caches.map.clear();
			};

			auto stageTypeInfo = StageTypeInfo {
				.storage = refl::PartialTypeInfo::from<system::MemoryStorage>(),
				.resource = refl::FullTypeInfo::from<Blob>(),
				.input = { nullptr }
			};

			_resolver.useStage<DynamicStage>(::hg::move(stageTypeInfo), ::hg::move(acqFn), ::hg::move(relFn));
		}

		// Upsert `ArchiveStorage` + `LfsSyncBlob` -(static)-> `StorageReadWriteArchive`
		{
			using stage0_type = decltype(StaticStage {
				.storage = Arci<system::ArchiveStorage> {},
				.acqFn = [](
				Context&,
				cref<Arci<ArchiveStorage>>,
				cref<IoResourceAccessor<LfsSyncBlob>>
			) -> stage_result_type<StorageReadWriteArchive> {
					EXPECT_FALSE(true);
					return Unexpected { AccessError { "Unintentional Failure." } };
				},
				.access = IoResourceAccessor<StorageReadWriteArchive> {},
				.relFn = [](Context&, cref<Arci<ArchiveStorage>>, IoResourceAccessor<StorageReadWriteArchive>&& val_) -> void {
					::hg::forward<decltype(val_)>(val_).reset();
				}
			});

			_resolver.useStage<stage0_type>(
				StageTypeInfo {
					.storage = refl::PartialTypeInfo::from<system::ArchiveStorage>(),
					.resource = refl::FullTypeInfo::from<StorageReadWriteArchive>(),
					.input = refl::FullTypeInfo::from<LfsSyncBlob>()
				}
			);
		}

		// Upsert `ArchiveStorage` + `const MemBlob` -(static)-> `StorageReadWriteArchive`
		{
			using stage0_type = decltype(StaticStage {
				.storage = Arci<system::ArchiveStorage> {},
				.acqFn = [](
				Context&,
				cref<Arci<system::ArchiveStorage>>,
				cref<IoResourceAccessor<ByteSpanBlob>>
			) -> stage_result_type<StorageReadWriteArchive> {
					EXPECT_TRUE(true);
					return Unexpected { AccessError { "Expected preserving result to exist control-flow for this test." } };
				},
				.access = IoResourceAccessor<StorageReadWriteArchive> {},
				.relFn = [](Context&, cref<Arci<ArchiveStorage>>, IoResourceAccessor<StorageReadWriteArchive>&& val_) -> void {
					::hg::forward<decltype(val_)>(val_).reset();
				}
			});

			_resolver.useStage<stage0_type>(
				StageTypeInfo {
					.storage = refl::PartialTypeInfo::from<system::ArchiveStorage>(),
					.resource = refl::FullTypeInfo::from<StorageReadWriteArchive>(),
					.input = refl::FullTypeInfo::from<ByteSpanBlob>()
				}
			);
		}

		// Request `ArchiveStorage` -> `StorageReadWriteArchive`
		auto signature = ActionTypeInfo {
			.targetResourceInfo = refl::FullTypeInfo::from<StorageReadWriteArchive>(),
			.stageTypeInfos = {
				StageTypeInfo {
					refl::PartialTypeInfo::from<system::ArchiveStorage>()
				},
				StageTypeInfo {
					refl::PartialTypeInfo::from<system::MemoryStorage>()
				}
			}
		};

		const auto factory = _resolver.resolveMutation(signature);
		ASSERT_TRUE(factory.has_value());

		/**/

		auto memDummy = Arci<system::MemoryStorage>::create(system::MemoryStorage::MemoryObject {}, true, true, true, true);
		auto archiveDummy = Arci<system::ArchiveStorage>::create(clone(memDummy).into<IStorage>(), true, true, true, true);

		auto mutation = (*factory)();
		ASSERT_TRUE(mutation);

		// Note: First internal type probe (`LfsSyncBlob`), generated while morphing, should fail and we escape with secondary path
		const auto preResult = mutation->prepare(clone(archiveDummy).into<IStorage>());
		ASSERT_TRUE(preResult.has_value());

		const auto acqResult = mutation->acquire(_context);
		ASSERT_FALSE(acqResult.has_value());

		ASSERT_TRUE(acqResult.error().is<AccessError>());

		EXPECT_TRUE(StringView {acqResult.error().as<AccessError>().what()}.starts_with("Expected preserving result"sv));

		// Note: Intermediate stages are discarded for type resolution tests...
		mutation->release(_context);
		mutation->cleanup();
	}
}
