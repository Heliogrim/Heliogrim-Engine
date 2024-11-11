#pragma once

#include <Engine.Common/Expected.hpp>
#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Managed/Rc.hpp>
#include <Engine.Resource.Archive/StorageReadonlyArchive.hpp>
#include <Engine.Resource.Archive/StorageReadWriteArchive.hpp>
#include <Engine.Resource.Package/ReadonlyPackage.hpp>
#include <Engine.Resource.Package/ReadWritePackage.hpp>
#include <Engine.Storage.Action/Context.hpp>
#include <Engine.Storage.Action/Mutation.hpp>
#include <Engine.Storage.Action/Query.hpp>
#include <Engine.Storage.Action/Access/IoResourceAccessor.hpp>
#include <Engine.Storage.Registry/Storage/ArchiveStorage.hpp>

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
