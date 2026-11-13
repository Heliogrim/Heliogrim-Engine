#pragma once

#include <Engine.Common/Expected.hpp>
#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Managed/Rc.hpp>
#include <Engine.Resource.Blob/Blob.hpp>
#include <Engine.Storage.Action/Context.hpp>
#include <Engine.Storage.Action/Mutation.hpp>
#include <Engine.Storage.Action/Query.hpp>
#include <Engine.Storage.Action/Access/IoResourceAccessor.hpp>
#include <Engine.Storage.Registry/Storage/LocalFileStorage.hpp>

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

	void releaseStaticLfsBlobQuery(
		_Inout_ ref<Context> ctx_,
		_In_ cref<Arci<system::LocalFileStorage>> storage_,
		_Inout_ mref<IoResourceAccessor<const resource::Blob>> accessor_
	);
}
