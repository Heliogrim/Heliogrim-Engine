#include "BlobPack.hpp"

#include <Engine.Common/__macro.hpp>

import Engine.Common;
import Engine.IO.Dispatcher;
import Engine.Logging;
import Engine.Pedantic.Resource;
import Engine.Storage.Action;
import Engine.Storage.Package;
import Engine.Storage.Registry;

using namespace hg::engine::storage;
using namespace hg::engine::resource;
using namespace hg;

namespace {
	[[nodiscard]] String renderAll(ref<const engine::diag::Diagnostics> diagnostics_);
}

/**/

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
		// Note: Built in memory only; release commits it, so a failed creation never leaves a partial package.
		value = Rc<ReadWritePackage>::create(package::make_read_write_package(accessor_.get()));

	} else {
		auto sink = diag::CollectingSink {};
		auto opened = package::open_read_write_package(accessor_.get(), storageName(*storage_), sink);
		if (not opened.has_value()) {
			return Result<IoResourceAccessor<ReadWritePackage>, Mutation::acq_error_type>::unexpected(
				IllegalArgumentError { renderAll(opened.error()) }
			);
		}

		value = Rc<ReadWritePackage>::create(::hg::move(opened).value());
	}

	auto cachedValue = ctx_.objectStore.add(
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
	auto storedResource = ctx_.accessStore.add(clone(storage_).into<IStorage>(), hg::move(resource));

	/**/

	return Expected(storedResource->acquireReadWrite());
}

Result<void, engine::io::CommitError> engine::storage::releaseStaticPackMutation(
	ref<Context> ctx_,
	cref<Arci<system::PackageStorage>> storage_,
	mref<IoResourceAccessor<resource::ReadWritePackage>> accessor_
) {
	auto deferred = ctx_.accessStore.drop(clone(storage_).into<IStorage>());

	/**/

	auto result = Result<void, io::CommitError> {};
	if (accessor_->getLinker().hasChanges() || accessor_->getLinker().getStorage().size() < 1) {

		auto stats = package::CommitStats {};
		const auto start = std::chrono::steady_clock::now();
		// The commit allocates plans and buffers; this release is noexcept, so out of memory becomes a commit error.
		try {
			result = package::commit_package_changes(accessor_.get(), &stats);
		} catch (const std::bad_alloc&) {
			// 8 = ERROR_NOT_ENOUGH_MEMORY; CommitError has no message field.
			result = Unexpected { io::CommitError { .kind = io::CommitError::Kind::eDeviceError, .osError = 8u } };
		}
		const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();

		// The commit outcome is decided; a log that throws (bad_alloc) must not end the process from this noexcept release.
		try {
			const auto name = storageName(*storage_);
			if (result.has_value()) {
				IM_CORE_INFOF(
					"Package commit `{}`: {} -> {} bytes, {} written, {} ms.",
					name,
					stats.oldSize,
					stats.newSize,
					stats.written,
					ms
				);
			} else {
				if (result.error().path.empty()) {
					result.error().path = name;
				}
				IM_CORE_ERRORF(
					"Package commit `{}` failed ({}): {} -> {} bytes, {} written, {} ms.",
					name,
					io::kindName(result.error().kind),
					stats.oldSize,
					stats.newSize,
					stats.written,
					ms
				);
			}
		} catch (...) {}
	}

	/**/

	ctx_.objectStore.invalidate(clone(storage_).into<IStorage>());
	::hg::forward<decltype(accessor_)>(accessor_).reset();
	::hg::discard(::hg::move(deferred));
	return result;
}

/**/

Result<
	IoResourceAccessor<const ReadonlyPackage>,
	Query::acq_error_type
> engine::storage::resolveStaticPackBlob2PackQuery(
	[[maybe_unused]] ref<Context> ctx_,
	[[maybe_unused]] cref<Arci<system::PackageStorage>> storage_,
	cref<IoResourceAccessor<const Blob>> accessor_
) {
	auto value = Rc<ReadonlyPackage> {};
	if (accessor_->size() < 1uLL) {
		value = Rc<ReadonlyPackage>::create(package::make_readonly_package(accessor_.get()));

	} else {
		auto sink = diag::CollectingSink {};
		auto opened = package::open_readonly_package(accessor_.get(), storageName(*storage_), sink);
		if (not opened.has_value()) {
			return Result<IoResourceAccessor<const ReadonlyPackage>, Query::acq_error_type>::unexpected(
				IllegalArgumentError { renderAll(opened.error()) }
			);
		}

		value = Rc<ReadonlyPackage>::create(::hg::move(opened).value());
	}

	/**/

	auto resolveReadonly = [_obj = hg::move(value)](ResourceAccessMode) -> ref<const ReadonlyPackage> {
		return *_obj;
	};
	auto resolveReadWrite = [](ResourceAccessMode) -> ref<ReadWritePackage> {
		panic();
	};

	return Expected(acquireSharedReadonly(::hg::move(resolveReadWrite), ::hg::move(resolveReadonly)));
}

void engine::storage::releaseStaticPackQuery(
	[[maybe_unused]] ref<Context> ctx_,
	[[maybe_unused]] cref<Arci<system::PackageStorage>> storage_,
	mref<IoResourceAccessor<const resource::ReadonlyPackage>> accessor_
) {
	// Note: The shared resource owns the object and dies with its last accessor.
	::hg::forward<decltype(accessor_)>(accessor_).reset();
}

/**/

namespace {
	String renderAll(ref<const engine::diag::Diagnostics> diagnostics_) {
		auto text = String {};
		for (const auto& entry : diagnostics_) {
			text += engine::diag::render(entry);
		}
		return text;
	}
}
