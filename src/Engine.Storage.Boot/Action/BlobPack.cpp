#include "BlobPack.hpp"

#include <Engine.Common/Discard.hpp>
#include <Engine.Pedantic.Resource/AccessMode.hpp>
#include <Engine.Storage.Action/Context.hpp>
#include <Engine.Storage.Action/Access/ExclusiveIoResource.hpp>
#include <Engine.Storage.Package/Factory.hpp>
#include <Engine.Storage.Package/Processor.hpp>

using namespace hg::engine::storage;
using namespace hg::engine::resource;
using namespace hg;

Result<
	IoResourceAccessor<ReadWritePackage>,
	Mutation::acq_error_type
> engine::storage::resolveStaticPackBlob2PackMutation(
	ref<Context> ctx_,
	cref<Arci<system::PackageStorage>> storage_,
	cref<IoResourceAccessor<Blob>> accessor_
) {
	using resolve_readonly_type = std::function<ref<const ReadonlyPackage>(ResourceAccessMode)>;
	using resolve_read_write_type = std::function<ref<ReadWritePackage>(ResourceAccessMode)>;

	using resource_type = decltype(ExclusiveIoResource {
		std::declval<resolve_read_write_type>(),
		std::declval<resolve_readonly_type>()
	});

	/**/

	auto value = Rc<ReadWritePackage> {};
	if (accessor_->size() < 1uLL) {
		value = Rc<ReadWritePackage>::create(package::make_read_write_package(accessor_.get()));

	} else {
		value = Rc<ReadWritePackage>::create(package::reconstruct_read_write_package(accessor_.get()));
	}

	auto cachedValue = ctx_.caches.add(
		clone(storage_).into<IStorage>(),
		hg::move(value),
		std::addressof(accessor_.get())
	);

	/**/

	auto resolveReadonly = [](ResourceAccessMode) -> ref<ReadonlyPackage> {
		panic();
	};
	auto resolveReadWrite = [_obj = hg::move(cachedValue)](ResourceAccessMode) mutable -> ref<ReadWritePackage> {
		return *_obj;
	};

	/**/

	auto resource = Rc<resource_type>::create(move(resolveReadWrite), move(resolveReadonly));
	auto storedResource = ctx_.store.add(clone(storage_).into<IStorage>(), hg::move(resource));

	/**/

	return Expected(storedResource->acquireReadWrite());
}

void engine::storage::releaseStaticPackMutation(
	ref<Context> ctx_,
	cref<Arci<system::PackageStorage>> storage_,
	mref<IoResourceAccessor<resource::ReadWritePackage>> accessor_
) {
	auto deferred = ctx_.store.drop(clone(storage_).into<IStorage>());

	/**/

	if (accessor_->getLinker().hasChanges()) {
		auto commitResult = package::commit_package_changes(accessor_.get());
		::hg::assertrt(commitResult.has_value());
	}

	/**/

	ctx_.caches.invalidate(clone(storage_).into<IStorage>());
	::hg::forward<decltype(accessor_)>(accessor_).reset();
	::hg::discard(::hg::move(deferred));
}

/**/

Result<
	IoResourceAccessor<const ReadonlyPackage>,
	Query::acq_error_type
> engine::storage::resolveStaticPackBlob2PackQuery(
	ref<Context> ctx_,
	cref<Arci<system::PackageStorage>> storage_,
	cref<IoResourceAccessor<const Blob>> accessor_
) {
	using resolve_readonly_type = std::function<ref<const ReadonlyPackage>(ResourceAccessMode)>;
	using resolve_read_write_type = std::function<ref<ReadWritePackage>(ResourceAccessMode)>;

	using resource_type = decltype(ExclusiveIoResource {
		std::declval<resolve_read_write_type>(),
		std::declval<resolve_readonly_type>()
	});

	/**/

	auto value = Rc<ReadonlyPackage> {};
	if (accessor_->size() < 1uLL) {
		value = Rc<ReadonlyPackage>::create(package::make_readonly_package(accessor_.get()));

	} else {
		value = Rc<ReadonlyPackage>::create(package::reconstruct_readonly_package(accessor_.get()));
	}

	auto cachedValue = ctx_.caches.add(
		clone(storage_).into<IStorage>(),
		hg::move(value),
		std::addressof(accessor_.get())
	);

	/**/

	auto resolveReadonly = [_obj = hg::move(cachedValue)](ResourceAccessMode) mutable -> ref<ReadonlyPackage> {
		return *_obj;
	};
	auto resolveReadWrite = [](ResourceAccessMode) -> ref<ReadWritePackage> {
		panic();
	};

	/**/

	auto resource = Rc<resource_type>::create(move(resolveReadWrite), move(resolveReadonly));
	auto storedResource = ctx_.store.add(clone(storage_).into<IStorage>(), hg::move(resource));

	/**/

	return Expected(storedResource->acquireReadonly());
}

void engine::storage::releaseStaticPackQuery(
	ref<Context> ctx_,
	cref<Arci<system::PackageStorage>> storage_,
	mref<IoResourceAccessor<const resource::ReadonlyPackage>> accessor_
) {
	auto deferred = ctx_.store.drop(clone(storage_).into<IStorage>());
	ctx_.caches.invalidate(clone(storage_).into<IStorage>());

	::hg::forward<decltype(accessor_)>(accessor_).reset();
	::hg::discard(::hg::move(deferred));
}
