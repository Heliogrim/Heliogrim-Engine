#pragma once

#include <Engine.Common/Sal.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
import Engine.Resource.Archive;
import Engine.Resource.Package;
import Engine.Storage.Action;
import Engine.Storage.Registry;
#endif

namespace hg::engine::storage {
	[[nodiscard]] Result<
		IoResourceAccessor<resource::Archive>,
		Mutation::acq_error_type
	> resolveStaticArchPack2ArchMutation(
		_Inout_ ref<Context> ctx_,
		_In_ cref<Arci<system::ArchiveStorage>> storage_,
		_In_ cref<IoResourceAccessor<resource::ReadWritePackage>> accessor_
	);

	void releaseStaticPackArchMutation(
		_Inout_ ref<Context> ctx_,
		_In_ cref<Arci<system::ArchiveStorage>> storage_,
		_Inout_ mref<IoResourceAccessor<resource::Archive>> accessor_
	);

	/**/

	[[nodiscard]] Result<
		IoResourceAccessor<resource::StorageReadonlyArchive>,
		Query::acq_error_type
	> resolveStaticArchPack2ArchQuery(
		_Inout_ ref<Context> ctx_,
		_In_ cref<Arci<system::ArchiveStorage>> storage_,
		_In_ cref<IoResourceAccessor<const resource::ReadonlyPackage>> accessor_
	);

	void releaseStaticPackArchQuery(
		_Inout_ ref<Context> ctx_,
		_In_ cref<Arci<system::ArchiveStorage>> storage_,
		_Inout_ mref<IoResourceAccessor<resource::StorageReadonlyArchive>> accessor_
	);
}
