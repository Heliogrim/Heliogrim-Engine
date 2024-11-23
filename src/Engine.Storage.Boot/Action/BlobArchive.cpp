#include "BlobArchive.hpp"

#include <Engine.Common/Discard.hpp>
#include <Engine.Pedantic.Resource/AccessMode.hpp>
#include <Engine.Storage.Action/Context.hpp>
#include <Engine.Storage.Action/Access/ExclusiveIoResource.hpp>

using namespace hg::engine::storage;
using namespace hg::engine::resource;
using namespace hg;

Result<
	IoResourceAccessor<StorageReadWriteArchive>,
	Mutation::acq_error_type
> engine::storage::resolveStaticArchMem2ArchMutation(
	ref<Context> ctx_,
	cref<Arci<system::ArchiveStorage>> storage_,
	cref<IoResourceAccessor<ByteSpanBlob>> accessor_
) {
	using resolve_readonly_type = std::function<ref<StorageReadonlyArchive>(ResourceAccessMode)>;
	using resolve_read_write_type = std::function<ref<StorageReadWriteArchive>(ResourceAccessMode)>;

	using resource_type = decltype(ExclusiveIoResource {
		std::declval<resolve_read_write_type>(),
		std::declval<resolve_readonly_type>()
	});

	/**/

	auto value = Rc<StorageReadWriteArchive>::create(accessor_.get(), streamoff {}, accessor_->size());
	auto cachedValue = ctx_.objectStore.add(
		clone(storage_).into<IStorage>(),
		::hg::move(value),
		std::addressof(accessor_.get())
	);

	/**/

	auto resolveReadonly = [](ResourceAccessMode) -> ref<StorageReadonlyArchive> {
		::hg::panic();
	};
	auto resolveReadWrite = [_obj = hg::move(cachedValue)](ResourceAccessMode) mutable -> ref<StorageReadWriteArchive> {
		return *_obj;
	};

	/**/

	auto resource = Rc<resource_type>::create(::hg::move(resolveReadWrite), ::hg::move(resolveReadonly));
	auto storedResource = ctx_.accessStore.add(clone(storage_).into<IStorage>(), ::hg::move(resource));

	/**/

	return Expected(storedResource->acquireReadWrite());

}

void engine::storage::releaseStaticArchMutation(
	ref<Context> ctx_,
	cref<Arci<system::ArchiveStorage>> storage_,
	mref<IoResourceAccessor<resource::StorageReadWriteArchive>> accessor_
) {
	auto deferred = ctx_.accessStore.drop(clone(storage_).into<IStorage>());
	ctx_.objectStore.invalidate(clone(storage_).into<IStorage>());

	::hg::forward<decltype(accessor_)>(accessor_).reset();
	::hg::discard(::hg::move(deferred));
}

/**/

Result<
	IoResourceAccessor<StorageReadonlyArchive>,
	Query::acq_error_type
> engine::storage::resolveStaticArchMem2ArchQuery(
	ref<Context> ctx_,
	cref<Arci<system::ArchiveStorage>> storage_,
	cref<IoResourceAccessor<const ByteSpanBlob>> accessor_
) {
	using resolve_readonly_type = std::function<ref<StorageReadonlyArchive>(ResourceAccessMode)>;
	using resolve_read_write_type = std::function<ref<StorageReadWriteArchive>(ResourceAccessMode)>;

	using resource_type = decltype(ExclusiveIoResource {
		std::declval<resolve_read_write_type>(),
		std::declval<resolve_readonly_type>()
	});

	/**/

	auto value = Rc<StorageReadonlyArchive>::create(accessor_.get(), streamoff {}, accessor_->size());
	auto cachedValue = ctx_.objectStore.add(
		clone(storage_).into<IStorage>(),
		::hg::move(value),
		std::addressof(accessor_.get())
	);

	/**/

	auto resolveReadonly = [_obj = hg::move(cachedValue)](ResourceAccessMode) mutable -> ref<StorageReadonlyArchive> {
		return *_obj;
	};
	auto resolveReadWrite = [](ResourceAccessMode) -> ref<StorageReadWriteArchive> {
		::hg::panic();
	};

	/**/

	auto resource = Rc<resource_type>::create(::hg::move(resolveReadWrite), ::hg::move(resolveReadonly));
	auto storedResource = ctx_.accessStore.add(clone(storage_).into<IStorage>(), ::hg::move(resource));

	/**/

	return Expected(storedResource->acquireReadonly());
}

void engine::storage::releaseStaticArchQuery(
	ref<Context> ctx_,
	cref<Arci<system::ArchiveStorage>> storage_,
	mref<IoResourceAccessor<resource::StorageReadonlyArchive>> accessor_
) {
	auto deferred = ctx_.accessStore.drop(clone(storage_).into<IStorage>());
	ctx_.objectStore.invalidate(clone(storage_).into<IStorage>());

	::hg::forward<decltype(accessor_)>(accessor_).reset();
	::hg::discard(::hg::move(deferred));
}
