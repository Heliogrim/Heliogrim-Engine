#pragma once

#include <Engine.Common/Sal.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
import Engine.Resource.Archive;
import Engine.Resource.Blob;
import Engine.Storage.Action;
import Engine.Storage.Registry;
#endif

namespace hg::engine::storage {
	[[nodiscard]] Result<
		IoResourceAccessor<resource::StorageReadWriteArchive>,
		Mutation::acq_error_type
	> resolveStaticArchMem2ArchMutation(
		_Inout_ ref<Context> ctx_,
		_In_ cref<Arci<system::ArchiveStorage>> storage_,
		_In_ cref<IoResourceAccessor<engine::resource::ByteSpanBlob>> accessor_
	);

	void releaseStaticArchMutation(
		_Inout_ ref<Context> ctx_,
		_In_ cref<Arci<system::ArchiveStorage>> storage_,
		_Inout_ mref<IoResourceAccessor<resource::StorageReadWriteArchive>> accessor_
	);

	/**/

	[[nodiscard]] Result<
		IoResourceAccessor<resource::StorageReadonlyArchive>,
		Query::acq_error_type
	> resolveStaticArchMem2ArchQuery(
		_Inout_ ref<Context> ctx_,
		_In_ cref<Arci<system::ArchiveStorage>> storage_,
		_In_ cref<IoResourceAccessor<const engine::resource::ByteSpanBlob>> accessor_
	);

	void releaseStaticArchQuery(
		_Inout_ ref<Context> ctx_,
		_In_ cref<Arci<system::ArchiveStorage>> storage_,
		_Inout_ mref<IoResourceAccessor<resource::StorageReadonlyArchive>> accessor_
	);
}
