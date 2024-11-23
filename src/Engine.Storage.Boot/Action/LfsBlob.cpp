#include "LfsBlob.hpp"

#include <Engine.Common/Discard.hpp>
#include <Engine.Pedantic.Resource/AccessMode.hpp>
#include <Engine.Storage.Action/Context.hpp>
#include <Engine.Storage.Action/Access/ExclusiveIoResource.hpp>

using namespace hg::engine::storage;
using namespace hg::engine::resource;
using namespace hg;

Result<IoResourceAccessor<LfsSyncBlob>, Mutation::acq_error_type> engine::storage::resolveStaticLfsVoid2LfsBlobMutation(
	ref<Context> ctx_,
	cref<Arci<system::LocalFileStorage>> storage_
) {
	using resolve_type = std::function<ref<LfsSyncBlob>(ResourceAccessMode)>;
	using resource_type = decltype(ExclusiveIoResource { std::declval<resolve_type>() });

	/**/

	auto value = Rc<LfsSyncBlob>::create(clone(storage_->_lfsPath));
	auto cachedValue = ctx_.objectStore.add(
		clone(storage_).into<IStorage>(),
		move(value),
		std::addressof(*storage_->_lfsPath.native().data())
	);

	/**/

	auto resolver = [_obj = move(cachedValue)](ResourceAccessMode) mutable -> ref<LfsSyncBlob> {
		return *_obj;
	};

	/**/

	auto resource = Rc<resource_type>::create(move(resolver));
	auto storedResource = ctx_.accessStore.add(clone(storage_).into<IStorage>(), hg::move(resource));

	/**/

	return Expected(storedResource->acquireReadWrite());
}

void engine::storage::releaseStaticLfsBlobMutation(
	ref<Context> ctx_,
	cref<Arci<system::LocalFileStorage>> storage_,
	mref<IoResourceAccessor<resource::LfsSyncBlob>> accessor_
) {
	auto deferred = ctx_.accessStore.drop(clone(storage_).into<IStorage>());
	ctx_.objectStore.invalidate(clone(storage_).into<IStorage>());

	::hg::forward<decltype(accessor_)>(accessor_).reset();
	::hg::discard(::hg::move(deferred));
}

/**/

Result<IoResourceAccessor<const LfsSyncBlob>, Query::acq_error_type> engine::storage::resolveStaticLfsVoid2LfsBlobQuery(
	ref<Context> ctx_,
	cref<Arci<system::LocalFileStorage>> storage_
) {
	using resolve_type = std::function<ref<LfsSyncBlob>(ResourceAccessMode)>;
	using resource_type = decltype(ExclusiveIoResource { std::declval<resolve_type>() });

	/**/

	auto value = Rc<LfsSyncBlob>::create(clone(storage_->_lfsPath));
	auto cachedValue = ctx_.objectStore.add(
		clone(storage_).into<IStorage>(),
		move(value),
		std::addressof(*storage_->_lfsPath.native().data())
	);

	/**/

	auto resolver = [_obj = move(cachedValue)](ResourceAccessMode) mutable -> ref<LfsSyncBlob> {
		return *_obj;
	};

	/**/

	auto resource = Rc<resource_type>::create(move(resolver));
	auto storedResource = ctx_.accessStore.add(clone(storage_).into<IStorage>(), hg::move(resource));

	/**/

	return Expected(storedResource->acquireReadonly());
}

void engine::storage::releaseStaticLfsBlobQuery(
	ref<Context> ctx_,
	cref<Arci<system::LocalFileStorage>> storage_,
	mref<IoResourceAccessor<const resource::LfsSyncBlob>> accessor_
) {
	auto deferred = ctx_.accessStore.drop(clone(storage_).into<IStorage>());
	ctx_.objectStore.invalidate(clone(storage_).into<IStorage>());

	::hg::forward<decltype(accessor_)>(accessor_).reset();
	::hg::discard(::hg::move(deferred));
}
