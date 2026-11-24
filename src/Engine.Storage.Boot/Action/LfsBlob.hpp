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
		IoResourceAccessor<engine::resource::Blob>,
		Mutation::acq_error_type
	> resolveStaticLfsVoid2LfsBlobMutation(
		_Inout_ ref<Context> ctx_,
		_In_ cref<Arci<system::LocalFileStorage>> storage_
	);

	void releaseStaticLfsBlobMutation(
		_Inout_ ref<Context> ctx_,
		_In_ cref<Arci<system::LocalFileStorage>> storage_,
		_Inout_ mref<IoResourceAccessor<resource::Blob>> accessor_
	);

	/**/

	[[nodiscard]] Result<
		IoResourceAccessor<const engine::resource::Blob>,
		Query::acq_error_type
	> resolveStaticLfsVoid2LfsBlobQuery(
		_Inout_ ref<Context> ctx_,
		_In_ cref<Arci<system::LocalFileStorage>> storage_
	);

	// The query above, opened unbuffered (Q03.7): the file stage under a package.
	[[nodiscard]] Result<
		IoResourceAccessor<const engine::resource::Blob>,
		Query::acq_error_type
	> resolveStaticLfsVoid2LfsBlobPackQuery(
		_Inout_ ref<Context> ctx_,
		_In_ ref<const Arci<system::LocalFileStorage>> storage_
	);

	void releaseStaticLfsBlobQuery(
		_Inout_ ref<Context> ctx_,
		_In_ cref<Arci<system::LocalFileStorage>> storage_,
		_Inout_ mref<IoResourceAccessor<const resource::Blob>> accessor_
	);
}
