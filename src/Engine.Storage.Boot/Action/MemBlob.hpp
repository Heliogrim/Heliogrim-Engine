#pragma once

#include <Engine.Common/Sal.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
import Engine.Resource.Blob;
import Engine.Storage.Action;
import Engine.Storage.Registry;
#endif

namespace hg::engine::storage {
	[[nodiscard]] Result<
		IoResourceAccessor<resource::ByteSpanBlob>,
		Mutation::acq_error_type
	> resolveStaticMemVoid2MemBlobMutation(
		_Inout_ ref<Context> ctx_,
		_In_ cref<Arci<system::MemoryStorage>> storage_
	);

	void releaseStaticMemBlobMutation(
		_Inout_ ref<Context> ctx_,
		_In_ cref<Arci<system::MemoryStorage>> storage_,
		_Inout_ mref<IoResourceAccessor<resource::ByteSpanBlob>> accessor_
	);

	/**/

	[[nodiscard]] Result<
		IoResourceAccessor<const resource::ByteSpanBlob>,
		Query::acq_error_type
	> resolveStaticMemVoid2MemBlobQuery(
		_Inout_ ref<Context> ctx_,
		_In_ cref<Arci<system::MemoryStorage>> storage_
	);

	void releaseStaticMemBlobQuery(
		_Inout_ ref<Context> ctx_,
		_In_ cref<Arci<system::MemoryStorage>> storage_,
		_Inout_ mref<IoResourceAccessor<const resource::ByteSpanBlob>> accessor_
	);
}
