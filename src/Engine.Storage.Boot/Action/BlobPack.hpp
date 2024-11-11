#pragma once

#include <Engine.Common/Expected.hpp>
#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Managed/Rc.hpp>
#include <Engine.Resource.Blob/Blob.hpp>
#include <Engine.Resource.Package/ReadonlyPackage.hpp>
#include <Engine.Resource.Package/ReadWritePackage.hpp>
#include <Engine.Storage.Action/Context.hpp>
#include <Engine.Storage.Action/Mutation.hpp>
#include <Engine.Storage.Action/Query.hpp>
#include <Engine.Storage.Action/Access/IoResourceAccessor.hpp>
#include <Engine.Storage.Registry/Storage/PackageStorage.hpp>

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
