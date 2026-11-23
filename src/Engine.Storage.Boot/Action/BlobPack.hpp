#pragma once

#include <Engine.Common/Sal.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
import Engine.Resource.Blob;
import Engine.Resource.Package;
import Engine.Storage.Action;
import Engine.Storage.Registry;
#endif

namespace hg::engine::storage {
	[[nodiscard]] Result<
		IoResourceAccessor<resource::ReadWritePackage>,
		Mutation::acq_error_type
	> resolveStaticPackBlob2PackMutation(
		_Inout_ ref<Context> ctx_,
		_In_ cref<Arci<system::PackageStorage>> storage_,
		_In_ cref<IoResourceAccessor<resource::Blob>> accessor_
	);

	void releaseStaticPackMutation(
		_Inout_ ref<Context> ctx_,
		_In_ cref<Arci<system::PackageStorage>> storage_,
		_Inout_ mref<IoResourceAccessor<resource::ReadWritePackage>> accessor_
	);

	/**/

	[[nodiscard]] Result<
		IoResourceAccessor<const resource::ReadonlyPackage>,
		Query::acq_error_type
	> resolveStaticPackBlob2PackQuery(
		_Inout_ ref<Context> ctx_,
		_In_ cref<Arci<system::PackageStorage>> storage_,
		_In_ cref<IoResourceAccessor<const resource::Blob>> accessor_
	);

	void releaseStaticPackQuery(
		_Inout_ ref<Context> ctx_,
		_In_ cref<Arci<system::PackageStorage>> storage_,
		_Inout_ mref<IoResourceAccessor<const resource::ReadonlyPackage>> accessor_
	);
}
