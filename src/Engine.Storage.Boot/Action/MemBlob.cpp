#include "MemBlob.hpp"

#include <Engine.Pedantic.Resource/AccessMode.hpp>
#include <Engine.Storage.Action/Context.hpp>
#include <Engine.Storage.Action/Access/ExclusiveIoResource.hpp>

#include "Discard.hpp"

using namespace hg::engine::storage;
using namespace hg::engine::resource;
using namespace hg;

Result<IoResourceAccessor<ByteSpanBlob>, Mutation::acq_error_type> engine::storage::resolveStaticMemVoid2MemBlobMutation(
	ref<Context> ctx_,
	cref<Arci<system::MemoryStorage>> storage_
) {
	using resolve_type = std::function<ref<ByteSpanBlob>(ResourceAccessMode)>;
	using resource_type = decltype(ExclusiveIoResource { std::declval<resolve_type>() });

	/**/

	auto& memory = storage_->_memory;

	auto value = Rc<ByteSpanBlob>::create(memory.span(), memory.touched());
	auto cachedValue = ctx_.caches.add(
		clone(storage_).into<IStorage>(),
		move(value),
		std::addressof(storage_->_memory)
	);

	/**/

	auto resolver = [_obj = move(cachedValue)](ResourceAccessMode) mutable -> ref<ByteSpanBlob> {
		return *_obj;
	};

	/**/

	auto resource = Rc<resource_type>::create(move(resolver));
	auto storedResource = ctx_.store.add(clone(storage_).into<IStorage>(), hg::move(resource));

	/**/

	return Expected(storedResource->acquireReadWrite());
}

void engine::storage::releaseStaticMemBlobMutation(
	ref<Context> ctx_,
	cref<Arci<system::MemoryStorage>> storage_,
	mref<IoResourceAccessor<resource::ByteSpanBlob>> accessor_
) {
	auto deferred = ctx_.store.drop(clone(storage_).into<IStorage>());

	/**/

	storage_->_memory.setAsTouched(accessor_->size());

	/**/

	ctx_.caches.invalidate(clone(storage_).into<IStorage>());
	::hg::forward<decltype(accessor_)>(accessor_).reset();
	::hg::discard(::hg::move(deferred));
}

/**/

Result<IoResourceAccessor<const ByteSpanBlob>, Query::acq_error_type> engine::storage::resolveStaticMemVoid2MemBlobQuery(
	ref<Context> ctx_,
	cref<Arci<system::MemoryStorage>> storage_
) {
	using resolve_type = std::function<ref<ByteSpanBlob>(ResourceAccessMode)>;
	using resource_type = decltype(ExclusiveIoResource { std::declval<resolve_type>() });

	/**/

	const auto& memory = storage_->_memory;

	auto value = Rc<ByteSpanBlob>::create(memory.touchedSpan(), memory.touched());
	auto cachedValue = ctx_.caches.add(
		clone(storage_).into<IStorage>(),
		move(value),
		std::addressof(storage_->_memory)
	);

	/**/

	auto resolver = [_obj = move(cachedValue)](ResourceAccessMode) mutable -> ref<ByteSpanBlob> {
		return *_obj;
	};

	/**/

	auto resource = Rc<resource_type>::create(move(resolver));
	auto storedResource = ctx_.store.add(clone(storage_).into<IStorage>(), hg::move(resource));

	/**/

	return Expected(storedResource->acquireReadonly());
}

void engine::storage::releaseStaticMemBlobQuery(
	ref<Context> ctx_,
	cref<Arci<system::MemoryStorage>> storage_,
	mref<IoResourceAccessor<const resource::ByteSpanBlob>> accessor_
) {
	auto deferred = ctx_.store.drop(clone(storage_).into<IStorage>());
	ctx_.caches.invalidate(clone(storage_).into<IStorage>());

	::hg::forward<decltype(accessor_)>(accessor_).reset();
	::hg::discard(::hg::move(deferred));
}
