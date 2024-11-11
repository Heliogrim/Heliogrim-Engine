#pragma once

#include <Engine.Common/Expected.hpp>
#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Managed/Rc.hpp>
#include <Engine.Resource.Blob/Memory/ByteSpanBlob.hpp>
#include <Engine.Storage.Action/Context.hpp>
#include <Engine.Storage.Action/Mutation.hpp>
#include <Engine.Storage.Action/Query.hpp>
#include <Engine.Storage.Action/Access/IoResourceAccessor.hpp>
#include <Engine.Storage.Registry/Storage/MemoryStorage.hpp>

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
