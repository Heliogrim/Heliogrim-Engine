#include "pch.h"

/**/

#include <Engine.Reflect/IsType.hpp>
#include <Engine.Resource.Archive/StorageReadonlyArchive.hpp>
#include <Engine.Resource.Blob/File/LfsSyncBlob.hpp>
#include <Engine.Resource.Blob/Memory/ByteSpanBlob.hpp>
#include <Engine.Resource.Package/Package.hpp>
#include <Engine.Storage.Action/Access/ExclusiveIoResource.hpp>
#include <Engine.Storage.Action/Action/QueryStaticGen.hpp>
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

	class QueryTypeResolutionFixture :
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
	TEST_F(QueryTypeResolutionFixture, FetchStrictStatic) {

		// TODO: Upsert `LocalFileStorage` + `null` -(static)-> `const LfsSyncBlob`
		// TODO: Upsert `PackageStorage` + `const LfsSyncBlob` -(static)-> `const Package`
		// TODO: Upsert `ArchiveStorage` + `const Package` -(static)-> `StorageReadonlyArchive`

		// TODO: Request `ArchiveStorage` -> `StorageReadonlyArchive`

		using namespace ::hg::engine::storage;

		/**/

		initActionResolver(_system, _resolver);

		/**/

		auto base = Arci<system::LocalFileStorage>::create(fs::Path {}, true, true, true, true);
		auto inter = Arci<system::PackageStorage>::create(clone(base).into<IStorage>(), true, true, true, true);
		auto dist = Arci<system::ArchiveStorage>::create(clone(inter).into<IStorage>(), true, true, true, true);

		/**/

		auto signature = ActionTypeInfo {
			.targetResourceInfo = refl::FullTypeInfo::from<engine::resource::StorageReadonlyArchive>(),
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

		//const auto result = system.query(
		//	clone(dist),
		//	[](ref<engine::resource::StorageReadonlyArchive> archive_) {
		//		archive_.flush();
		//	}
		//);
		const auto factory = _resolver.resolveQuery(signature);
		EXPECT_TRUE(factory.has_value());

		auto query = (*factory)();

		const auto preResult = query->prepare(clone(dist).into<IStorage>());
		//const auto acqResult = query->acquire(context);
		//query->exec([](const auto&) -> void {});
		//query->release(context);
		query->cleanup();
	}

	TEST_F(QueryTypeResolutionFixture, FetchCovariantStatic) {
		// Note: Do not upsert any static stages or actions

		// Upsert `LocalFileStorage` + `null` -> `const LfsSyncBlob`
		using stage0_type = decltype(StaticStage {
			.storage = Arci<system::LocalFileStorage> {},
			.acqFn = [](Context&, cref<Arci<system::LocalFileStorage>>) -> stage_result_type<const LfsSyncBlob> {
				return Unexpected { AccessError { "Intentional Failure." } };
			},
			.access = IoResourceAccessor<const LfsSyncBlob> {},
			.relFn = [](Context&, cref<Arci<LocalFileStorage>>, IoResourceAccessor<const LfsSyncBlob>&& val_) -> void {
				::hg::forward<decltype(val_)>(val_).reset();
			}
		});
		using query_type = QueryStaticGen<stage0_type>;

		auto actionTypeInfo = ActionTypeInfo {
			.targetResourceInfo = refl::FullTypeInfo::from<const LfsSyncBlob>(),
			.stageTypeInfos = {
				StageTypeInfo {
					.storage = refl::PartialTypeInfo::from<system::LocalFileStorage>(),
					.resource = refl::FullTypeInfo::from<const LfsSyncBlob>(),
					.input = {}
				}
			}
		};

		_resolver.use<query_type>(::hg::move(actionTypeInfo));

		// Request `LocalFileStorage` -> `const Blob`
		auto signature = ActionTypeInfo {
			.targetResourceInfo = refl::FullTypeInfo::from<const Blob>(),
			.stageTypeInfos = {
				StageTypeInfo {
					refl::PartialTypeInfo::from<system::LocalFileStorage>()
				}
			}
		};

		const auto factory = _resolver.resolveQuery(signature);
		ASSERT_TRUE(factory.has_value());

		/**/

		auto dummy = Arci<system::LocalFileStorage>::create(fs::Path {}, true, true, true, true);

		auto query = (*factory)();

		const auto preResult = query->prepare(clone(dummy).into<IStorage>());
		EXPECT_TRUE(preResult.has_value());
		// Note: Intermediate stages are discarded for type resolution tests...
		query->cleanup();
	}

	TEST_F(QueryTypeResolutionFixture, FetchStrictDynamic) {
		// Note: Do not upsert any static stages or actions

		// Upsert `LocalFileStorage` + `null` -(dyn)-> `const LfsSyncBlob`
		{
			decltype(DynamicStage::acqFn) acqFn = [](Context&, cref<Arci<IStorage>>, const auto&) -> auto {
				return Unexpected<Query::acq_error_type> { AccessError { "Intentional Failure." } };
			};
			decltype(DynamicStage::relFn) relFn = [](Context&, cref<Arci<IStorage>>, auto&& val_) -> void {
				::hg::forward<decltype(val_)>(val_).value.reset();
			};

			auto stageTypeInfo = StageTypeInfo {
				.storage = refl::PartialTypeInfo::from<system::LocalFileStorage>(),
				.resource = refl::FullTypeInfo::from<const LfsSyncBlob>(),
				.input = {}
			};

			_resolver.useStage<DynamicStage>(::hg::move(stageTypeInfo), ::hg::move(acqFn), ::hg::move(relFn));
		}

		// Upsert `PackageStorage` + `const LfsSyncBlob` -(dyn)-> `const Package`
		{
			decltype(DynamicStage::acqFn) acqFn = [](Context&, cref<Arci<IStorage>>, const auto&) -> auto {
				return Unexpected<Query::acq_error_type> { AccessError { "Intentional Failure." } };
			};
			decltype(DynamicStage::relFn) relFn = [](Context&, cref<Arci<IStorage>>, auto&& val_) -> void {
				::hg::forward<decltype(val_)>(val_).value.reset();
			};

			auto stageTypeInfo = StageTypeInfo {
				.storage = refl::PartialTypeInfo::from<system::PackageStorage>(),
				.resource = refl::FullTypeInfo::from<const Package>(),
				.input = refl::FullTypeInfo::from<const LfsSyncBlob>()
			};

			_resolver.useStage<DynamicStage>(::hg::move(stageTypeInfo), ::hg::move(acqFn), ::hg::move(relFn));
		}

		// Upsert `ArchiveStorage` + `const Package` -(dyn)-> `StorageReadonlyArchive`
		{
			decltype(DynamicStage::acqFn) acqFn = [](Context&, cref<Arci<IStorage>>, const auto&) -> auto {
				return Unexpected<Query::acq_error_type> { AccessError { "Intentional Failure." } };
			};
			decltype(DynamicStage::relFn) relFn = [](Context&, cref<Arci<IStorage>>, auto&& val_) -> void {
				::hg::forward<decltype(val_)>(val_).value.reset();
			};

			auto stageTypeInfo = StageTypeInfo {
				.storage = refl::PartialTypeInfo::from<system::ArchiveStorage>(),
				.resource = refl::FullTypeInfo::from<StorageReadonlyArchive>(),
				.input = refl::FullTypeInfo::from<const Package>()
			};

			_resolver.useStage<DynamicStage>(::hg::move(stageTypeInfo), ::hg::move(acqFn), ::hg::move(relFn));
		}

		// Request `ArchiveStorage` -> `StorageReadonlyArchive`
		auto signature = ActionTypeInfo {
			.targetResourceInfo = refl::FullTypeInfo::from<StorageReadonlyArchive>(),
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

		const auto factory = _resolver.resolveQuery(signature);
		ASSERT_TRUE(factory.has_value());

		/**/

		auto lfsDummy = Arci<system::LocalFileStorage>::create(fs::Path {}, true, true, true, true);
		auto packageDummy = Arci<system::PackageStorage>::create(clone(lfsDummy).into<IStorage>(), true, true, true, true);
		auto archiveDummy = Arci<system::ArchiveStorage>::create(clone(packageDummy).into<IStorage>(), true, true, true, true);

		auto query = (*factory)();
		ASSERT_TRUE(query);

		const auto preResult = query->prepare(clone(archiveDummy).into<IStorage>());
		EXPECT_TRUE(preResult.has_value());
		// Note: Intermediate stages are discarded for type resolution tests...
		query->cleanup();
	}

	TEST_F(QueryTypeResolutionFixture, FetchCovariantDynamic) {
		// Note: Do not upsert any static stages or actions

		// Upsert `LocalFileStorage` + `null` -(dyn)-> `const LfsSyncBlob`
		decltype(DynamicStage::acqFn) acqFn = [](Context&, cref<Arci<IStorage>>, const auto&) -> auto {
			return Unexpected<Query::acq_error_type> { AccessError { "Intentional Failure." } };
		};
		decltype(DynamicStage::relFn) relFn = [](Context&, cref<Arci<IStorage>>, auto&& val_) -> void {
			::hg::forward<decltype(val_)>(val_).value.reset();
		};

		auto stageTypeInfo = StageTypeInfo {
			.storage = refl::PartialTypeInfo::from<system::LocalFileStorage>(),
			.resource = refl::FullTypeInfo::from<const LfsSyncBlob>(),
			.input = {}
		};

		_resolver.useStage<DynamicStage>(::hg::move(stageTypeInfo), ::hg::move(acqFn), ::hg::move(relFn));

		// Request `LocalFileStorage` -> `const Blob`
		auto signature = ActionTypeInfo {
			.targetResourceInfo = refl::FullTypeInfo::from<const Blob>(),
			.stageTypeInfos = {
				StageTypeInfo {
					refl::PartialTypeInfo::from<system::LocalFileStorage>()
				}
			}
		};

		const auto factory = _resolver.resolveQuery(signature);
		ASSERT_TRUE(factory.has_value());

		/**/

		auto dummy = Arci<system::LocalFileStorage>::create(fs::Path {}, true, true, true, true);

		auto query = (*factory)();
		ASSERT_TRUE(query);

		const auto preResult = query->prepare(clone(dummy).into<IStorage>());
		EXPECT_TRUE(preResult.has_value());
		// Note: Intermediate stages are discarded for type resolution tests...
		query->cleanup();
	}

	TEST_F(QueryTypeResolutionFixture, FetchMorphedDynamic) {
		// Note: Do not upsert any static stages or actions

		// Upsert `LocalFileStorage` + `null` -(static)-> `const LfsSyncBlob`
		using static_stage_type = decltype(StaticStage {
			.storage = Arci<system::LocalFileStorage> {},
			.acqFn = [](Context&, cref<Arci<system::LocalFileStorage>>) -> stage_result_type<const LfsSyncBlob> {
				return Unexpected { AccessError { "Intentional Failure." } };
			},
			.access = IoResourceAccessor<const LfsSyncBlob> {},
			.relFn = [](Context&, cref<Arci<LocalFileStorage>>, IoResourceAccessor<const LfsSyncBlob>&& val_) -> void {
				::hg::forward<decltype(val_)>(val_).reset();
			}
		});

		auto stageTypeInfo = StageTypeInfo {
			.storage = refl::PartialTypeInfo::from<system::LocalFileStorage>(),
			.resource = refl::FullTypeInfo::from<const LfsSyncBlob>(),
			.input = {}
		};

		_resolver.useStage<static_stage_type>(::hg::move(stageTypeInfo));

		// Upsert `ArchiveStorage` + `const LfsSyncBlob` -(dyn)-> `StorageReadonlyArchive` // Note: Required to force morphing
		{
			decltype(DynamicStage::acqFn) acqFn = [](Context&, cref<Arci<IStorage>>, const auto&) -> auto {
				return Unexpected<Query::acq_error_type> { AccessError { "Intentional Failure." } };
			};
			decltype(DynamicStage::relFn) relFn = [](Context&, cref<Arci<IStorage>>, auto&& val_) -> void {
				::hg::forward<decltype(val_)>(val_).value.reset();
			};

			auto stageTypeInfo = StageTypeInfo {
				.storage = refl::PartialTypeInfo::from<system::ArchiveStorage>(),
				.resource = refl::FullTypeInfo::from<StorageReadonlyArchive>(),
				.input = refl::FullTypeInfo::from<const LfsSyncBlob>()
			};

			_resolver.useStage<DynamicStage>(::hg::move(stageTypeInfo), ::hg::move(acqFn), ::hg::move(relFn));
		}

		// Request `ArchiveStorage` -> `StorageReadonlyArchive`
		auto signature = ActionTypeInfo {
			.targetResourceInfo = refl::FullTypeInfo::from<StorageReadonlyArchive>(),
			.stageTypeInfos = {
				StageTypeInfo {
					refl::PartialTypeInfo::from<system::ArchiveStorage>()
				},
				StageTypeInfo {
					refl::PartialTypeInfo::from<system::LocalFileStorage>()
				}
			}
		};

		const auto factory = _resolver.resolveQuery(signature);
		ASSERT_TRUE(factory.has_value());

		/**/

		auto lfsDummy = Arci<system::LocalFileStorage>::create(fs::Path {}, true, true, true, true);
		auto archiveDummy = Arci<system::ArchiveStorage>::create(clone(lfsDummy).into<IStorage>(), true, true, true, true);

		auto query = (*factory)();
		ASSERT_TRUE(query);

		const auto preResult = query->prepare(clone(archiveDummy).into<IStorage>());
		EXPECT_TRUE(preResult.has_value());
		// Note: Intermediate stages are discarded for type resolution tests...
		query->cleanup();
	}

	TEST_F(QueryTypeResolutionFixture, FetchCovariantDynamicComposed) {
		// Note: Do not upsert any static stages or actions

		// Upsert `MemoryStorage` + `null` -(dyn)-> `const Blob`
		{
			decltype(DynamicStage::acqFn) acqFn = [](
				Context& ctx_,
				cref<Arci<IStorage>> storage_,
				const auto& input_
			) -> Result<meta::WithType<Rc<IoResourceAccessorBase>>, Query::acq_error_type> {

				auto memStore = storage_.into<system::MemoryStorage>();
				if (not IsType<system::MemoryStorage>(*memStore)) {
					return Unexpected<Query::acq_error_type> { IllegalTypeError { "Invalid storage type at dynamic query stage." } };
				}

				if (input_.value != nullptr) {
					return Unexpected<Query::acq_error_type> {
						IllegalArgumentError { "Illegally provided an input resource to a primary stage transformer." }
					};
				}

				/**/

				using resolve_type = std::function<ref<engine::resource::ByteSpanBlob>(ResourceAccessMode)>;
				using resource_type = decltype(ExclusiveIoResource { std::declval<resolve_type>() });

				/**/

				auto value = Rc<engine::resource::ByteSpanBlob>::create(memStore->_memory.span());
				auto cachedValue = ctx_.objectStore.add(clone(storage_), ::hg::move(value));

				/**/

				auto resolver = [_obj = ::hg::move(cachedValue)](ResourceAccessMode) mutable -> ref<engine::resource::ByteSpanBlob> {
					return *_obj;
				};

				/**/

				auto resource = Rc<resource_type>::create(::hg::move(resolver));
				auto storedResource = ctx_.accessStore.add(::hg::move(memStore).into<IStorage>(), ::hg::move(resource));

				/**/

				return Expected(
					meta::WithType<Rc<IoResourceAccessorBase>>::from(
						Rc<IoResourceAccessor<const ByteSpanBlob>>::create(storedResource->acquireReadonly())
					)
				);
			};
			decltype(DynamicStage::relFn) relFn = [](Context&, cref<Arci<IStorage>>, auto&& val_) -> void {
				::hg::forward<decltype(val_)>(val_).value.reset();
			};

			auto stageTypeInfo = StageTypeInfo {
				.storage = refl::PartialTypeInfo::from<system::MemoryStorage>(),
				.resource = refl::FullTypeInfo::from<const Blob>(),
				.input = {}
			};

			_resolver.useStage<DynamicStage>(::hg::move(stageTypeInfo), ::hg::move(acqFn), ::hg::move(relFn));
		}

		// Upsert `ArchiveStorage` + `const LfsSyncBlob` -(dyn)-> `StorageReadonlyArchive`
		{
			decltype(DynamicStage::acqFn) acqFn = [](Context&, cref<Arci<IStorage>>, const auto& input_) -> auto {
				const auto& maybeCasted = reinterpret_cast<cref<meta::WithType<Rc<ResourceAccessor<const LfsSyncBlob>>>>>(input_);
				if (not maybeCasted.valid()) {
					return Unexpected<Query::acq_error_type> { IllegalTypeError { "Expected type Failure." } };
				}
				return Unexpected<Query::acq_error_type> { AccessError { "Unintentional Failure." } };
			};
			decltype(DynamicStage::relFn) relFn = [](Context&, cref<Arci<IStorage>>, auto&& val_) -> void {
				::hg::forward<decltype(val_)>(val_).value.reset();
			};

			auto stageTypeInfo = StageTypeInfo {
				.storage = refl::PartialTypeInfo::from<system::ArchiveStorage>(),
				.resource = refl::FullTypeInfo::from<StorageReadonlyArchive>(),
				.input = refl::FullTypeInfo::from<const LfsSyncBlob>()
			};

			_resolver.useStage<DynamicStage>(::hg::move(stageTypeInfo), ::hg::move(acqFn), ::hg::move(relFn));
		}

		// Upsert `ArchiveStorage` + `const MemBlob` -(dyn)-> `StorageReadonlyArchive`
		{
			decltype(DynamicStage::acqFn) acqFn = [](Context&, cref<Arci<IStorage>>, const auto& input_) -> auto {
				const auto& maybeCasted = reinterpret_cast<cref<meta::WithType<Rc<IoResourceAccessor<const ByteSpanBlob>>>>>(input_);
				if (not maybeCasted.valid()) {
					return Unexpected<Query::acq_error_type> { IllegalTypeError { "Unintentional Failure." } };
				}
				return Unexpected<Query::acq_error_type> {
					AccessError { "Expected preserving result to exist control-flow for this test." }
				};
			};
			decltype(DynamicStage::relFn) relFn = [](Context&, cref<Arci<IStorage>>, auto&& val_) -> void {
				::hg::forward<decltype(val_)>(val_).value.reset();
			};

			auto stageTypeInfo = StageTypeInfo {
				.storage = refl::PartialTypeInfo::from<system::ArchiveStorage>(),
				.resource = refl::FullTypeInfo::from<StorageReadonlyArchive>(),
				.input = refl::FullTypeInfo::from<const ByteSpanBlob>()
			};

			_resolver.useStage<DynamicStage>(::hg::move(stageTypeInfo), ::hg::move(acqFn), ::hg::move(relFn));
		}

		// Request `ArchiveStorage` -> `StorageReadonlyArchive`
		auto signature = ActionTypeInfo {
			.targetResourceInfo = refl::FullTypeInfo::from<StorageReadonlyArchive>(),
			.stageTypeInfos = {
				StageTypeInfo {
					refl::PartialTypeInfo::from<system::ArchiveStorage>()
				},
				StageTypeInfo {
					refl::PartialTypeInfo::from<system::MemoryStorage>()
				}
			}
		};

		const auto factory = _resolver.resolveQuery(signature);
		ASSERT_TRUE(factory.has_value());

		/**/

		auto memDummy = Arci<MemoryStorage>::create(system::MemoryStorage::MemoryObject {}, true, true, true, true);
		auto archiveDummy = Arci<ArchiveStorage>::create(clone(memDummy).into<IStorage>(), ArchiveGuid {}, true, true, true, true);

		auto query = (*factory)();
		ASSERT_TRUE(query);

		// Note: First internal type probe (`LfsSyncBlob`) should fail and we escape with secondary path
		const auto preResult = query->prepare(clone(archiveDummy).into<IStorage>());
		ASSERT_TRUE(preResult.has_value());

		const auto acqResult = query->acquire(_context);
		ASSERT_FALSE(acqResult.has_value());

		ASSERT_TRUE(acqResult.error().is<AccessError>());
		EXPECT_TRUE(StringView {acqResult.error().as<AccessError>().what()}.starts_with("Expected preserving result"sv));

		// Note: Intermediate stages are discarded for type resolution tests...
		query->release(_context);
		query->cleanup();
	}

	TEST_F(QueryTypeResolutionFixture, FetchMorphedCovariantDynamicComposed) {
		// Note: Do not upsert any static stages or actions

		// Upsert `MemoryStorage` + `null` -(dyn)-> `const Blob`
		{
			decltype(DynamicStage::acqFn) acqFn = [](
				Context& ctx_,
				cref<Arci<IStorage>> storage_,
				const auto& input_
			) -> Result<meta::WithType<Rc<IoResourceAccessorBase>>, Query::acq_error_type> {

				auto memStore = storage_.into<system::MemoryStorage>();
				if (not IsType<system::MemoryStorage>(*memStore)) {
					return Unexpected<Query::acq_error_type> {
						IllegalTypeError { "Invalid storage type at dynamic query stage." }
					};
				}

				if (input_.value != nullptr) {
					return Unexpected<Query::acq_error_type> {
						IllegalArgumentError { "Illegally provided an input resource to a primary stage transformer." }
					};
				}

				/**/

				using resolve_type = std::function<ref<engine::resource::ByteSpanBlob>(ResourceAccessMode)>;
				using resource_type = decltype(ExclusiveIoResource { std::declval<resolve_type>() });

				/**/

				auto value = Rc<engine::resource::ByteSpanBlob>::create(memStore->_memory.span());
				auto cachedValue = ctx_.objectStore.add(clone(storage_), ::hg::move(value));

				/**/

				auto resolver = [_obj = ::hg::move(cachedValue)](ResourceAccessMode) mutable -> ref<engine::resource::ByteSpanBlob> {
					return *_obj;
				};

				/**/

				auto resource = Rc<resource_type>::create(::hg::move(resolver));
				auto storedResource = ctx_.accessStore.add(::hg::move(memStore).into<IStorage>(), ::hg::move(resource));

				/**/

				return Expected(
					meta::WithType<Rc<IoResourceAccessorBase>>::from(
						Rc<IoResourceAccessor<const ByteSpanBlob>>::create(storedResource->acquireReadonly())
					)
				);
			};
			decltype(DynamicStage::relFn) relFn = [](Context& ctx_, cref<Arci<IStorage>>, auto&& val_) -> void {
				::hg::forward<decltype(val_)>(val_).value.reset();

				ctx_.accessStore.map.clear();
				ctx_.objectStore.map.clear();
			};

			auto stageTypeInfo = StageTypeInfo {
				.storage = refl::PartialTypeInfo::from<system::MemoryStorage>(),
				.resource = refl::FullTypeInfo::from<const Blob>(),
				.input = {}
			};

			_resolver.useStage<DynamicStage>(::hg::move(stageTypeInfo), ::hg::move(acqFn), ::hg::move(relFn));
		}

		// Upsert `ArchiveStorage` + `const LfsSyncBlob` -(static)-> `StorageReadonlyArchive`
		{
			using stage0_type = decltype(StaticStage {
				.storage = Arci<system::ArchiveStorage> {},
				.acqFn = [](
				Context&,
				cref<Arci<system::ArchiveStorage>>,
				cref<IoResourceAccessor<const LfsSyncBlob>>
			) -> stage_result_type<StorageReadonlyArchive> {
					EXPECT_FALSE(true);
					return Unexpected { AccessError { "Unintentional Failure." } };
				},
				.access = IoResourceAccessor<StorageReadonlyArchive> {},
				.relFn = [](Context&, cref<Arci<ArchiveStorage>>, IoResourceAccessor<StorageReadonlyArchive>&& val_) -> void {
					::hg::forward<decltype(val_)>(val_).reset();
				}
			});

			_resolver.useStage<stage0_type>(
				StageTypeInfo {
					.storage = refl::PartialTypeInfo::from<system::ArchiveStorage>(),
					.resource = refl::FullTypeInfo::from<StorageReadonlyArchive>(),
					.input = refl::FullTypeInfo::from<const LfsSyncBlob>()
				}
			);
		}

		// Upsert `ArchiveStorage` + `const MemBlob` -(static)-> `StorageReadonlyArchive`
		{
			using stage0_type = decltype(StaticStage {
				.storage = Arci<system::ArchiveStorage> {},
				.acqFn = [](
				Context&,
				cref<Arci<system::ArchiveStorage>>,
				cref<IoResourceAccessor<const ByteSpanBlob>>
			) -> stage_result_type<StorageReadonlyArchive> {
					EXPECT_TRUE(true);
					return Unexpected { AccessError { "Expected preserving result to exist control-flow for this test." } };
				},
				.access = IoResourceAccessor<StorageReadonlyArchive> {},
				.relFn = [](Context&, cref<Arci<ArchiveStorage>>, IoResourceAccessor<StorageReadonlyArchive>&& val_) -> void {
					::hg::forward<decltype(val_)>(val_).reset();
				}
			});

			_resolver.useStage<stage0_type>(
				StageTypeInfo {
					.storage = refl::PartialTypeInfo::from<system::ArchiveStorage>(),
					.resource = refl::FullTypeInfo::from<StorageReadonlyArchive>(),
					.input = refl::FullTypeInfo::from<const ByteSpanBlob>()
				}
			);
		}

		// Request `ArchiveStorage` -> `StorageReadonlyArchive`
		auto signature = ActionTypeInfo {
			.targetResourceInfo = refl::FullTypeInfo::from<StorageReadonlyArchive>(),
			.stageTypeInfos = {
				StageTypeInfo {
					refl::PartialTypeInfo::from<system::ArchiveStorage>()
				},
				StageTypeInfo {
					refl::PartialTypeInfo::from<system::MemoryStorage>()
				}
			}
		};

		const auto factory = _resolver.resolveQuery(signature);
		ASSERT_TRUE(factory.has_value());

		/**/

		auto memDummy = Arci<system::MemoryStorage>::create(system::MemoryStorage::MemoryObject {}, true, true, true, true);
		auto archiveDummy = Arci<system::ArchiveStorage>::create(clone(memDummy).into<IStorage>(), true, true, true, true);

		auto query = (*factory)();
		ASSERT_TRUE(query);

		// Note: First internal type probe (`LfsSyncBlob`), generated while morphing, should fail and we escape with secondary path
		const auto preResult = query->prepare(clone(archiveDummy).into<IStorage>());
		ASSERT_TRUE(preResult.has_value());

		const auto acqResult = query->acquire(_context);
		ASSERT_FALSE(acqResult.has_value());

		ASSERT_TRUE(acqResult.error().is<AccessError>());

		EXPECT_TRUE(StringView {acqResult.error().as<AccessError>().what()}.starts_with("Expected preserving result"sv));

		// Note: Intermediate stages are discarded for type resolution tests...
		query->release(_context);
		query->cleanup();
	}
}
