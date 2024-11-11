#include "ActionResolverInit.hpp"

#include <Engine.Resource.Archive/StorageReadonlyArchive.hpp>
#include <Engine.Resource.Archive/StorageReadWriteArchive.hpp>
#include <Engine.Resource.Blob/Blob.hpp>
#include <Engine.Resource.Blob/File/LfsSyncBlob.hpp>
#include <Engine.Resource.Package/ReadonlyPackage.hpp>
#include <Engine.Resource.Package/ReadWritePackage.hpp>
#include <Engine.Storage.Action/Action/MutationStaticGen.hpp>
#include <Engine.Storage.Action/Action/QueryStaticGen.hpp>
#include <Engine.Storage.Registry/Storage/ArchiveStorage.hpp>
#include <Engine.Storage.Registry/Storage/LocalFileStorage.hpp>
#include <Engine.Storage.Registry/Storage/PackageStorage.hpp>
#include <Engine.Storage.System/StorageSystem.hpp>
#include <Engine.Storage.System/Details/StorageActionResolver.hpp>

#include "BlobArchive.hpp"
#include "BlobPack.hpp"
#include "LfsBlob.hpp"
#include "MemBlob.hpp"
#include "PackArchive.hpp"

using namespace hg::engine::storage;
using namespace hg;

/**/

// Problem: Using static functions may result in un-emitted function bodies, as the compiler will drop the instantiation
// Problem:		due to the lack of calling references...

/**/

void engine::storage::initActionResolver(ref<StorageSystem> system_, ref<StorageActionResolver> resolver_) {

	using staticLfsVoid2LfsBlobMut = decltype(system::StaticStage {
		.storage = Arci<system::LocalFileStorage> {},
		.acqFn = [](auto&&... args_) {
			return resolveStaticLfsVoid2LfsBlobMutation(::hg::forward<decltype(args_)>(args_)...);
		},
		.access = IoResourceAccessor<resource::LfsSyncBlob> {},
		.relFn = [](auto&&... args_) {
			return releaseStaticLfsBlobMutation(::hg::forward<decltype(args_)>(args_)...);
		}
	});

	using staticLfsVoid2LfsBlobQuery = decltype(system::StaticStage {
		.storage = Arci<system::LocalFileStorage> {},
		.acqFn = [](auto&&... args_) {
			return resolveStaticLfsVoid2LfsBlobQuery(::hg::forward<decltype(args_)>(args_)...);
		},
		.access = IoResourceAccessor<const resource::LfsSyncBlob> {},
		.relFn = [](auto&&... args_) {
			return releaseStaticLfsBlobQuery(::hg::forward<decltype(args_)>(args_)...);
		}
	});

	/**/

	using staticMemVoid2MemBlobMut = decltype(system::StaticStage {
		.storage = Arci<system::MemoryStorage> {},
		.acqFn = [](auto&&... args_) {
			return resolveStaticMemVoid2MemBlobMutation(::hg::forward<decltype(args_)>(args_)...);
		},
		.access = IoResourceAccessor<resource::ByteSpanBlob> {},
		.relFn = [](auto&&... args_) {
			return releaseStaticMemBlobMutation(::hg::forward<decltype(args_)>(args_)...);
		}
	});

	using staticMemVoid2MemBlobQuery = decltype(system::StaticStage {
		.storage = Arci<system::MemoryStorage> {},
		.acqFn = [](auto&&... args_) {
			return resolveStaticMemVoid2MemBlobQuery(::hg::forward<decltype(args_)>(args_)...);
		},
		.access = IoResourceAccessor<const resource::ByteSpanBlob> {},
		.relFn = [](auto&&... args_) {
			return releaseStaticMemBlobQuery(::hg::forward<decltype(args_)>(args_)...);
		}
	});

	/**/

	using staticArchMem2ArchMut = decltype(system::StaticStage {
		.storage = Arci<system::ArchiveStorage> {},
		.acqFn = [](auto&&... args_) {
			return resolveStaticArchMem2ArchMutation(::hg::forward<decltype(args_)>(args_)...);
		},
		.access = IoResourceAccessor<resource::StorageReadWriteArchive> {},
		.relFn = [](auto&&... args_) {
			return releaseStaticArchMutation(::hg::forward<decltype(args_)>(args_)...);
		}
	});

	using staticArchMem2ArchQuery = decltype(system::StaticStage {
		.storage = Arci<system::ArchiveStorage> {},
		.acqFn = [](auto&&... args_) {
			return resolveStaticArchMem2ArchQuery(::hg::forward<decltype(args_)>(args_)...);
		},
		.access = IoResourceAccessor<resource::StorageReadonlyArchive> {},
		.relFn = [](auto&&... args_) {
			return releaseStaticArchQuery(::hg::forward<decltype(args_)>(args_)...);
		}
	});

	/**/

	using staticPackBlob2PackMut = decltype(system::StaticStage {
		.storage = Arci<system::PackageStorage> {},
		.acqFn = [](auto&&... args_) {
			return resolveStaticPackBlob2PackMutation(::hg::forward<decltype(args_)>(args_)...);
		},
		.access = IoResourceAccessor<resource::ReadWritePackage> {},
		.relFn = [](auto&&... args_) {
			return releaseStaticPackMutation(::hg::forward<decltype(args_)>(args_)...);
		}
	});

	using staticPackLfsBlob2PackMut = decltype(system::StaticStage {
		.storage = Arci<system::PackageStorage> {},
		.acqFn = [](auto&& ctx_, auto&& storage_, const auto& accessor_) {
			return resolveStaticPackBlob2PackMutation(
				::hg::forward<decltype(ctx_)>(ctx_),
				::hg::forward<decltype(storage_)>(storage_),
				reinterpret_cast<cref<IoResourceAccessor<resource::Blob>>>(accessor_)
			);
		},
		.access = IoResourceAccessor<resource::ReadWritePackage> {},
		.relFn = [](auto&&... args_) {
			return releaseStaticPackMutation(::hg::forward<decltype(args_)>(args_)...);
		}
	});

	using staticPackMemBlob2PackMut = decltype(system::StaticStage {
		.storage = Arci<system::PackageStorage> {},
		.acqFn = [](auto&& ctx_, auto&& storage_, const auto& accessor_) {
			return resolveStaticPackBlob2PackMutation(
				::hg::forward<decltype(ctx_)>(ctx_),
				::hg::forward<decltype(storage_)>(storage_),
				reinterpret_cast<cref<IoResourceAccessor<resource::Blob>>>(accessor_)
			);
		},
		.access = IoResourceAccessor<resource::ReadWritePackage> {},
		.relFn = [](auto&&... args_) {
			return releaseStaticPackMutation(::hg::forward<decltype(args_)>(args_)...);
		}
	});

	using staticPackBlob2PackQuery = decltype(system::StaticStage {
		.storage = Arci<system::PackageStorage> {},
		.acqFn = [](auto&&... args_) {
			return resolveStaticPackBlob2PackQuery(::hg::forward<decltype(args_)>(args_)...);
		},
		.access = IoResourceAccessor<const resource::ReadonlyPackage> {},
		.relFn = [](auto&&... args_) {
			return releaseStaticPackQuery(::hg::forward<decltype(args_)>(args_)...);
		}
	});

	using staticPackLfsBlob2PackQuery = decltype(system::StaticStage {
		.storage = Arci<system::PackageStorage> {},
		.acqFn = [](auto&& ctx_, auto&& storage_, const auto& accessor_) {
			return resolveStaticPackBlob2PackQuery(
				::hg::forward<decltype(ctx_)>(ctx_),
				::hg::forward<decltype(storage_)>(storage_),
				reinterpret_cast<cref<IoResourceAccessor<const resource::Blob>>>(accessor_)
			);
		},
		.access = IoResourceAccessor<const resource::ReadonlyPackage> {},
		.relFn = [](auto&&... args_) {
			return releaseStaticPackQuery(::hg::forward<decltype(args_)>(args_)...);
		}
	});

	using staticPackMemBlob2PackQuery = decltype(system::StaticStage {
		.storage = Arci<system::PackageStorage> {},
		.acqFn = [](auto&& ctx_, auto&& storage_, const auto& accessor_) {
			return resolveStaticPackBlob2PackQuery(
				::hg::forward<decltype(ctx_)>(ctx_),
				::hg::forward<decltype(storage_)>(storage_),
				reinterpret_cast<cref<IoResourceAccessor<const resource::Blob>>>(accessor_)
			);
		},
		.access = IoResourceAccessor<const resource::ReadonlyPackage> {},
		.relFn = [](auto&&... args_) {
			return releaseStaticPackQuery(::hg::forward<decltype(args_)>(args_)...);
		}
	});

	/**/

	using staticArchPack2ArchMut = decltype(system::StaticStage {
		.storage = Arci<system::ArchiveStorage> {},
		.acqFn = [](auto&&... args_) {
			return resolveStaticArchPack2ArchMutation(::hg::forward<decltype(args_)>(args_)...);
		},
		.access = IoResourceAccessor<resource::Archive> {},
		.relFn = [](auto&&... args_) {
			return releaseStaticPackArchMutation(::hg::forward<decltype(args_)>(args_)...);
		}
	});

	using staticArchPack2ArchQuery = decltype(system::StaticStage {
		.storage = Arci<system::ArchiveStorage> {},
		.acqFn = [](auto&&... args_) {
			return resolveStaticArchPack2ArchQuery(::hg::forward<decltype(args_)>(args_)...);
		},
		.access = IoResourceAccessor<resource::StorageReadonlyArchive> {},
		.relFn = [](auto&&... args_) {
			return releaseStaticPackArchQuery(::hg::forward<decltype(args_)>(args_)...);
		}
	});

	/**/

	{
		using mutation_type = system::MutationStaticGen<
			staticLfsVoid2LfsBlobMut
		>;

		auto actionTypeInfo = ActionTypeInfo {
			.targetResourceInfo = refl::FullTypeInfo::from<resource::LfsSyncBlob>(),
			.stageTypeInfos = {
				StageTypeInfo {
					.storage = refl::PartialTypeInfo::from<system::LocalFileStorage>(),
					.resource = refl::FullTypeInfo::from<resource::LfsSyncBlob>(),
					.input = { nullptr }
				},
			}
		};

		resolver_.use<mutation_type>(::hg::move(actionTypeInfo));
	}

	/**/

	{
		using query_type = system::QueryStaticGen<
			staticLfsVoid2LfsBlobQuery
		>;

		auto actionTypeInfo = ActionTypeInfo {
			.targetResourceInfo = refl::FullTypeInfo::from<const resource::LfsSyncBlob>(),
			.stageTypeInfos = {
				StageTypeInfo {
					.storage = refl::PartialTypeInfo::from<system::LocalFileStorage>(),
					.resource = refl::FullTypeInfo::from<const resource::LfsSyncBlob>(),
					.input = { nullptr }
				},
			}
		};

		resolver_.use<query_type>(::hg::move(actionTypeInfo));
	}

	/**/

	{
		using mutation_type = system::MutationStaticGen<
			staticMemVoid2MemBlobMut
		>;

		auto actionTypeInfo = ActionTypeInfo {
			.targetResourceInfo = refl::FullTypeInfo::from<resource::ByteSpanBlob>(),
			.stageTypeInfos = {
				StageTypeInfo {
					.storage = refl::PartialTypeInfo::from<system::MemoryStorage>(),
					.resource = refl::FullTypeInfo::from<resource::ByteSpanBlob>(),
					.input = { nullptr }
				},
			}
		};

		resolver_.use<mutation_type>(::hg::move(actionTypeInfo));
	}

	/**/

	{
		using query_type = system::QueryStaticGen<
			staticMemVoid2MemBlobQuery
		>;

		auto actionTypeInfo = ActionTypeInfo {
			.targetResourceInfo = refl::FullTypeInfo::from<const resource::ByteSpanBlob>(),
			.stageTypeInfos = {
				StageTypeInfo {
					.storage = refl::PartialTypeInfo::from<system::MemoryStorage>(),
					.resource = refl::FullTypeInfo::from<const resource::ByteSpanBlob>(),
					.input = { nullptr }
				},
			}
		};

		resolver_.use<query_type>(::hg::move(actionTypeInfo));
	}

	/**/

	{
		using mutation_type = system::QueryStaticGen<
			staticArchMem2ArchMut,
			staticMemVoid2MemBlobMut
		>;

		auto actionTypeInfo = ActionTypeInfo {
			.targetResourceInfo = refl::FullTypeInfo::from<resource::StorageReadWriteArchive>(),
			.stageTypeInfos = {
				StageTypeInfo {
					.storage = refl::PartialTypeInfo::from<system::ArchiveStorage>(),
					.resource = refl::FullTypeInfo::from<resource::StorageReadWriteArchive>(),
					.input = refl::FullTypeInfo::from<resource::ByteSpanBlob>()
				},
				StageTypeInfo {
					.storage = refl::PartialTypeInfo::from<system::MemoryStorage>(),
					.resource = refl::FullTypeInfo::from<resource::ByteSpanBlob>(),
					.input = { nullptr }
				},
			}
		};

		resolver_.use<mutation_type>(::hg::move(actionTypeInfo));
	}

	/**/

	{
		using query_type = system::QueryStaticGen<
			staticArchMem2ArchQuery,
			staticMemVoid2MemBlobQuery
		>;

		auto actionTypeInfo = ActionTypeInfo {
			.targetResourceInfo = refl::FullTypeInfo::from<resource::StorageReadonlyArchive>(),
			.stageTypeInfos = {
				StageTypeInfo {
					.storage = refl::PartialTypeInfo::from<system::ArchiveStorage>(),
					.resource = refl::FullTypeInfo::from<resource::StorageReadonlyArchive>(),
					.input = refl::FullTypeInfo::from<const resource::ByteSpanBlob>()
				},
				StageTypeInfo {
					.storage = refl::PartialTypeInfo::from<system::MemoryStorage>(),
					.resource = refl::FullTypeInfo::from<const resource::ByteSpanBlob>(),
					.input = { nullptr }
				},
			}
		};

		resolver_.use<query_type>(::hg::move(actionTypeInfo));
	}

	/**/

	{
		using mutation_type = system::MutationStaticGen<
			staticPackMemBlob2PackMut,
			staticMemVoid2MemBlobMut
		>;

		auto actionTypeInfo = ActionTypeInfo {
			.targetResourceInfo = refl::FullTypeInfo::from<resource::ReadWritePackage>(),
			.stageTypeInfos = {
				StageTypeInfo {
					.storage = refl::PartialTypeInfo::from<system::PackageStorage>(),
					.resource = refl::FullTypeInfo::from<resource::ReadWritePackage>(),
					.input = refl::FullTypeInfo::from<resource::ByteSpanBlob>()
				},
				StageTypeInfo {
					.storage = refl::PartialTypeInfo::from<system::MemoryStorage>(),
					.resource = refl::FullTypeInfo::from<resource::ByteSpanBlob>(),
					.input = { nullptr }
				},
			}
		};

		resolver_.use<mutation_type>(::hg::move(actionTypeInfo));
	}

	/**/

	{
		using mutation_type = system::MutationStaticGen<
			staticPackLfsBlob2PackMut,
			staticLfsVoid2LfsBlobMut
		>;

		auto actionTypeInfo = ActionTypeInfo {
			.targetResourceInfo = refl::FullTypeInfo::from<resource::ReadWritePackage>(),
			.stageTypeInfos = {
				StageTypeInfo {
					.storage = refl::PartialTypeInfo::from<system::PackageStorage>(),
					.resource = refl::FullTypeInfo::from<resource::ReadWritePackage>(),
					.input = refl::FullTypeInfo::from<resource::LfsSyncBlob>()
				},
				StageTypeInfo {
					.storage = refl::PartialTypeInfo::from<system::LocalFileStorage>(),
					.resource = refl::FullTypeInfo::from<resource::LfsSyncBlob>(),
					.input = { nullptr }
				},
			}
		};

		resolver_.use<mutation_type>(::hg::move(actionTypeInfo));
	}

	/**/

	{
		using query_type = system::QueryStaticGen<
			staticPackMemBlob2PackQuery,
			staticMemVoid2MemBlobQuery
		>;

		auto actionTypeInfo = ActionTypeInfo {
			.targetResourceInfo = refl::FullTypeInfo::from<const resource::ReadonlyPackage>(),
			.stageTypeInfos = {
				StageTypeInfo {
					.storage = refl::PartialTypeInfo::from<system::PackageStorage>(),
					.resource = refl::FullTypeInfo::from<const resource::ReadonlyPackage>(),
					.input = refl::FullTypeInfo::from<const resource::ByteSpanBlob>()
				},
				StageTypeInfo {
					.storage = refl::PartialTypeInfo::from<system::MemoryStorage>(),
					.resource = refl::FullTypeInfo::from<const resource::ByteSpanBlob>(),
					.input = { nullptr }
				},
			}
		};

		resolver_.use<query_type>(::hg::move(actionTypeInfo));
	}

	/**/

	{
		using query_type = system::QueryStaticGen<
			staticPackLfsBlob2PackQuery,
			staticLfsVoid2LfsBlobQuery
		>;

		auto actionTypeInfo = ActionTypeInfo {
			.targetResourceInfo = refl::FullTypeInfo::from<const resource::ReadonlyPackage>(),
			.stageTypeInfos = {
				StageTypeInfo {
					.storage = refl::PartialTypeInfo::from<system::PackageStorage>(),
					.resource = refl::FullTypeInfo::from<const resource::ReadonlyPackage>(),
					.input = refl::FullTypeInfo::from<const resource::LfsSyncBlob>()
				},
				StageTypeInfo {
					.storage = refl::PartialTypeInfo::from<system::LocalFileStorage>(),
					.resource = refl::FullTypeInfo::from<const resource::LfsSyncBlob>(),
					.input = { nullptr }
				},
			}
		};

		resolver_.use<query_type>(::hg::move(actionTypeInfo));
	}

	/**/

	{
		using mutation_type = system::MutationStaticGen<
			staticArchPack2ArchMut,
			staticPackMemBlob2PackMut,
			staticMemVoid2MemBlobMut
		>;

		auto actionTypeInfo = ActionTypeInfo {
			.targetResourceInfo = refl::FullTypeInfo::from<resource::Archive>(),
			.stageTypeInfos = {
				StageTypeInfo {
					.storage = refl::PartialTypeInfo::from<system::ArchiveStorage>(),
					.resource = refl::FullTypeInfo::from<resource::Archive>(),
					.input = refl::FullTypeInfo::from<resource::ReadWritePackage>()
				},
				StageTypeInfo {
					.storage = refl::PartialTypeInfo::from<system::PackageStorage>(),
					.resource = refl::FullTypeInfo::from<resource::ReadWritePackage>(),
					.input = refl::FullTypeInfo::from<resource::ByteSpanBlob>()
				},
				StageTypeInfo {
					.storage = refl::PartialTypeInfo::from<system::MemoryStorage>(),
					.resource = refl::FullTypeInfo::from<resource::ByteSpanBlob>(),
					.input = { nullptr }
				},
			}
		};

		resolver_.use<mutation_type>(::hg::move(actionTypeInfo));
	}

	/**/

	{
		using mutation_type = system::MutationStaticGen<
			staticArchPack2ArchMut,
			staticPackLfsBlob2PackMut,
			staticLfsVoid2LfsBlobMut
		>;

		auto actionTypeInfo = ActionTypeInfo {
			.targetResourceInfo = refl::FullTypeInfo::from<resource::Archive>(),
			.stageTypeInfos = {
				StageTypeInfo {
					.storage = refl::PartialTypeInfo::from<system::ArchiveStorage>(),
					.resource = refl::FullTypeInfo::from<resource::Archive>(),
					.input = refl::FullTypeInfo::from<resource::ReadWritePackage>()
				},
				StageTypeInfo {
					.storage = refl::PartialTypeInfo::from<system::PackageStorage>(),
					.resource = refl::FullTypeInfo::from<resource::ReadWritePackage>(),
					.input = refl::FullTypeInfo::from<resource::LfsSyncBlob>()
				},
				StageTypeInfo {
					.storage = refl::PartialTypeInfo::from<system::LocalFileStorage>(),
					.resource = refl::FullTypeInfo::from<resource::LfsSyncBlob>(),
					.input = { nullptr }
				},
			}
		};

		resolver_.use<mutation_type>(::hg::move(actionTypeInfo));
	}

	/**/

	{
		using query_type = system::QueryStaticGen<
			staticArchPack2ArchQuery,
			staticPackMemBlob2PackQuery,
			staticMemVoid2MemBlobQuery
		>;

		auto actionTypeInfo = ActionTypeInfo {
			.targetResourceInfo = refl::FullTypeInfo::from<resource::StorageReadonlyArchive>(),
			.stageTypeInfos = {
				StageTypeInfo {
					.storage = refl::PartialTypeInfo::from<system::ArchiveStorage>(),
					.resource = refl::FullTypeInfo::from<resource::StorageReadonlyArchive>(),
					.input = refl::FullTypeInfo::from<const resource::ReadonlyPackage>()
				},
				StageTypeInfo {
					.storage = refl::PartialTypeInfo::from<system::PackageStorage>(),
					.resource = refl::FullTypeInfo::from<const resource::ReadonlyPackage>(),
					.input = refl::FullTypeInfo::from<const resource::ByteSpanBlob>()
				},
				StageTypeInfo {
					.storage = refl::PartialTypeInfo::from<system::MemoryStorage>(),
					.resource = refl::FullTypeInfo::from<const resource::ByteSpanBlob>(),
					.input = { nullptr }
				},
			}
		};

		resolver_.use<query_type>(::hg::move(actionTypeInfo));
	}

	/**/

	{
		using query_type = system::QueryStaticGen<
			staticArchPack2ArchQuery,
			staticPackLfsBlob2PackQuery,
			staticLfsVoid2LfsBlobQuery
		>;

		auto actionTypeInfo = ActionTypeInfo {
			.targetResourceInfo = refl::FullTypeInfo::from<resource::StorageReadonlyArchive>(),
			.stageTypeInfos = {
				StageTypeInfo {
					.storage = refl::PartialTypeInfo::from<system::ArchiveStorage>(),
					.resource = refl::FullTypeInfo::from<resource::StorageReadonlyArchive>(),
					.input = refl::FullTypeInfo::from<const resource::ReadonlyPackage>()
				},
				StageTypeInfo {
					.storage = refl::PartialTypeInfo::from<system::PackageStorage>(),
					.resource = refl::FullTypeInfo::from<const resource::ReadonlyPackage>(),
					.input = refl::FullTypeInfo::from<const resource::LfsSyncBlob>()
				},
				StageTypeInfo {
					.storage = refl::PartialTypeInfo::from<system::LocalFileStorage>(),
					.resource = refl::FullTypeInfo::from<const resource::LfsSyncBlob>(),
					.input = { nullptr }
				},
			}
		};

		resolver_.use<query_type>(::hg::move(actionTypeInfo));
	}

	/**/
}
