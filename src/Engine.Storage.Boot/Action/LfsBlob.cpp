#include "LfsBlob.hpp"

#include <filesystem>
#include <Engine.Asserts/Asserts.hpp>
#include <Engine.Async/Affine/AffineThread.hpp>
#include <Engine.Common/Discard.hpp>
#include <Engine.Pedantic.Resource/AccessMode.hpp>
#include <Engine.Resource.Blob/File/IoFileBlob.hpp>
#include <Engine.Resource.Blob/File/LfsSyncBlob.hpp>
#include <Engine.Storage.Action/Context.hpp>
#include <Engine.Storage.Action/Access/AccessError.hpp>
#include <Engine.Storage.Action/Access/ExclusiveIoResource.hpp>
#include <Engine.Storage.Action/Access/SharedReadIoResource.hpp>

using namespace hg::engine::storage;
using namespace hg::engine::resource;
using namespace hg;

namespace {
	// Null when the io thread cannot open the file.
	[[nodiscard]] Rc<Blob> openBlob(ref<Context> ctx_, cref<Arci<system::LocalFileStorage>> storage_, engine::io::IoOpenMode mode_) {

		if (ctx_.io == nullptr || not ctx_.io->running()) {
			return Rc<LfsSyncBlob>::create(clone(storage_->_lfsPath));
		}

		// Blocking on the io thread from an affine thread would stall its own drain loop.
		::hg::assertd(engine::async::AffineThread::current() == nullptr);

		auto opened = await(ctx_.io->open(static_cast<std::filesystem::path>(storage_->_lfsPath), mode_));
		if (not opened.has_value()) {
			return nullptr;
		}

		// Note: One queue per blob; the blob owns it and releases it with the file.
		return Rc<IoFileBlob>::create(*ctx_.io, ::hg::move(opened).value(), ctx_.io->createQueue(engine::io::IoPriority::eNormal));
	}
}

Result<IoResourceAccessor<Blob>, Mutation::acq_error_type> engine::storage::resolveStaticLfsVoid2LfsBlobMutation(
	ref<Context> ctx_,
	cref<Arci<system::LocalFileStorage>> storage_
) {
	using resolve_type = std::function<ref<Blob>(ResourceAccessMode)>;
	using resource_type = decltype(ExclusiveIoResource { std::declval<resolve_type>() });

	/**/

	auto value = openBlob(ctx_, storage_, engine::io::IoOpenMode::eReadWrite);
	if (value == nullptr) {
		return Unexpected<Mutation::acq_error_type> { AccessError { "Failed to open file for mutation." } };
	}

	auto cachedValue = ctx_.objectStore.add(
		clone(storage_).into<IStorage>(),
		move(value),
		std::addressof(*storage_->_lfsPath.native().data())
	);

	/**/

	auto resolver = [_obj = move(cachedValue)](ResourceAccessMode) mutable -> ref<Blob> {
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
	mref<IoResourceAccessor<resource::Blob>> accessor_
) {
	auto deferred = ctx_.accessStore.drop(clone(storage_).into<IStorage>());
	ctx_.objectStore.invalidate(clone(storage_).into<IStorage>());

	::hg::forward<decltype(accessor_)>(accessor_).reset();
	::hg::discard(::hg::move(deferred));
}

/**/

Result<IoResourceAccessor<const Blob>, Query::acq_error_type> engine::storage::resolveStaticLfsVoid2LfsBlobQuery(
	ref<Context> ctx_,
	cref<Arci<system::LocalFileStorage>> storage_
) {
	auto value = openBlob(ctx_, storage_, engine::io::IoOpenMode::eRead);
	if (value == nullptr) {
		return Unexpected<Query::acq_error_type> { AccessError { "Failed to open file for query." } };
	}

	return Expected(
		acquireSharedReadonly(
			[_obj = move(value)](ResourceAccessMode) -> ref<const Blob> {
				return *_obj;
			}
		)
	);
}

void engine::storage::releaseStaticLfsBlobQuery(
	[[maybe_unused]] ref<Context> ctx_,
	[[maybe_unused]] cref<Arci<system::LocalFileStorage>> storage_,
	mref<IoResourceAccessor<const resource::Blob>> accessor_
) {
	// Note: The shared resource owns the object and dies with its last accessor.
	::hg::forward<decltype(accessor_)>(accessor_).reset();
}
