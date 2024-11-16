#include "PackArchive.hpp"

#include <Engine.Asserts/Todo.hpp>
#include <Engine.Common/Discard.hpp>
#include <Engine.Pedantic.Resource/AccessMode.hpp>
#include <Engine.Storage.Action/Context.hpp>
#include <Engine.Storage.Action/Access/ExclusiveIoResource.hpp>

using namespace hg::engine::storage;
using namespace hg::engine::resource;
using namespace hg;

Result<
	IoResourceAccessor<Archive>,
	Mutation::acq_error_type
> engine::storage::resolveStaticArchPack2ArchMutation(
	ref<Context> ctx_,
	cref<Arci<system::ArchiveStorage>> storage_,
	cref<IoResourceAccessor<ReadWritePackage>> accessor_
) {
	using resolve_readonly_type = std::function<ref<StorageReadonlyArchive>(ResourceAccessMode)>;
	using resolve_read_write_type = std::function<ref<Archive>(ResourceAccessMode)>;

	using resource_type = decltype(ExclusiveIoResource {
		std::declval<resolve_read_write_type>(),
		std::declval<resolve_readonly_type>()
	});

	/**/

	// TODO: We need to get the guid to query for the correct linker data.
	::hg::assertrt(accessor_->getLinker().count() <= 1uLL);
	auto archGuid = ArchiveGuid {};

	auto& linker = accessor_->getLinker();
	//const auto& linked = linker.get(ArchiveGuid {});
	auto& linked = *linker.begin();

	/**/

	auto value = Rc<StorageReadWriteArchive> {};
	if (archGuid != linked.second.header.guid) {
		auto& nextLinked = linker.add({ .type = ArchiveType::eUndefined, .guid = archGuid });
		// nextLinked.changes;
		value = Rc<StorageReadWriteArchive>::create(
			linker.getStorage(),
			nextLinked.data.offset,
			nextLinked.data.size
		);
		::hg::todo_panic();

	} else {
		auto maybeLinked = linker.get(archGuid);
		// maybeLinked.changes;
		value = Rc<StorageReadWriteArchive>::create(
			linker.getStorage(),
			maybeLinked->data.offset,
			maybeLinked->data.size
		);
	}

	auto cachedValue = ctx_.caches.add(
		clone(storage_).into<IStorage>(),
		::hg::move(value),
		std::addressof(accessor_.get())
	);

	/**/

	auto resolveReadonly = [](ResourceAccessMode) -> ref<StorageReadonlyArchive> {
		::hg::panic();
	};
	auto resolveReadWrite = [_obj = hg::move(cachedValue)](ResourceAccessMode) mutable -> ref<Archive> {
		return *_obj;
	};

	/**/

	auto resource = Rc<resource_type>::create(::hg::move(resolveReadWrite), ::hg::move(resolveReadonly));
	auto storedResource = ctx_.store.add(clone(storage_).into<IStorage>(), ::hg::move(resource));

	/**/

	return Expected(storedResource->acquireReadWrite());

}

void engine::storage::releaseStaticPackArchMutation(
	ref<Context> ctx_,
	cref<Arci<system::ArchiveStorage>> storage_,
	mref<IoResourceAccessor<resource::Archive>> accessor_
) {
	auto deferred = ctx_.store.drop(clone(storage_).into<IStorage>());
	ctx_.caches.invalidate(clone(storage_).into<IStorage>());

	::hg::forward<decltype(accessor_)>(accessor_).reset();
	::hg::discard(::hg::move(deferred));
}

/**/

Result<
	IoResourceAccessor<StorageReadonlyArchive>,
	Query::acq_error_type
> engine::storage::resolveStaticArchPack2ArchQuery(
	ref<Context> ctx_,
	cref<Arci<system::ArchiveStorage>> storage_,
	cref<IoResourceAccessor<const ReadonlyPackage>> accessor_
) {
	using resolve_readonly_type = std::function<ref<StorageReadonlyArchive>(ResourceAccessMode)>;
	using resolve_read_write_type = std::function<ref<StorageReadWriteArchive>(ResourceAccessMode)>;

	using resource_type = decltype(ExclusiveIoResource {
		std::declval<resolve_read_write_type>(),
		std::declval<resolve_readonly_type>()
	});

	/**/

	// TODO: We need to get the guid to query for the correct linker data.
	::hg::assertrt(accessor_->getLinker().count() <= 1uLL);
	auto archGuid = ArchiveGuid {};

	auto& linker = accessor_->getLinker();
	//const auto& linked = linker.get(ArchiveGuid {});
	auto& linked = *linker.begin();

	/**/

	if (archGuid != linked.second.header.guid) {
		return Unexpected<Query::acq_error_type> { AccessError { "Requested archive does not exist within package." } };

	}

	auto maybeLinked = linker.get(archGuid);
	// maybeLinked.changes;
	auto value = Rc<StorageReadonlyArchive>::create(
		linker.getStorage(),
		maybeLinked->data.offset,
		maybeLinked->data.size
	);

	auto cachedValue = ctx_.caches.add(
		clone(storage_).into<IStorage>(),
		::hg::move(value),
		std::addressof(accessor_.get()),
		std::addressof(accessor_->getLinker().getStorage())
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
	auto storedResource = ctx_.store.add(clone(storage_).into<IStorage>(), ::hg::move(resource));

	/**/

	return Expected(storedResource->acquireReadonly());
}

void engine::storage::releaseStaticPackArchQuery(
	ref<Context> ctx_,
	cref<Arci<system::ArchiveStorage>> storage_,
	mref<IoResourceAccessor<resource::StorageReadonlyArchive>> accessor_
) {
	auto deferred = ctx_.store.drop(clone(storage_).into<IStorage>());
	ctx_.caches.invalidate(clone(storage_).into<IStorage>());

	::hg::forward<decltype(accessor_)>(accessor_).reset();
	::hg::discard(::hg::move(deferred));
}
