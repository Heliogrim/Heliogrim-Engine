#include "PackArchive.hpp"

#include <Engine.Asserts/Todo.hpp>
#include <Engine.Common/Discard.hpp>
#include <Engine.Pedantic.Resource/AccessMode.hpp>
#include <Engine.Reflect/IsType.hpp>
#include <Engine.Resource.Archive/BufferArchive.hpp>
#include <Engine.Resource.Archive/Storage/PackageReadWriteArchive.hpp>
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

	const auto archiveGuid = storage_->getArchiveGuid();
	::hg::assertd(archiveGuid != ArchiveGuid {});

	auto& linker = accessor_->getLinker();
	auto linked = linker.get(archiveGuid);

	/**/

	auto value = Rc<PackageReadWriteArchive> {};
	if (linked == None) {
		auto& nextLinked = linker.add({ .type = ArchiveType::eUndefined, .guid = archiveGuid });
		value = Rc<PackageReadWriteArchive>::create(accessor_.get(), nextLinked);

	} else {
		value = Rc<PackageReadWriteArchive>::create(accessor_.get(), linked.value());
		value->preload(linked->data.size);
	}

	// Warning: Experimental control-flow
	auto cachedValue = ctx_.objectStore.add(
		// Attention: Attach object to parent (Package Storage) invalidation cycle.
		//clone(storage_).into<IStorage>(),
		storage_->getBacking(),
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
	auto storedResource = ctx_.accessStore.add(clone(storage_).into<IStorage>(), ::hg::move(resource));

	/**/

	return Expected(storedResource->acquireReadWrite());
}

void engine::storage::releaseStaticPackArchMutation(
	ref<Context> ctx_,
	cref<Arci<system::ArchiveStorage>> storage_,
	mref<IoResourceAccessor<resource::Archive>> accessor_
) {
	auto deferred = ctx_.accessStore.drop(clone(storage_).into<IStorage>());
	ctx_.objectStore.invalidate(clone(storage_).into<IStorage>());

	/**/

	if (IsType<PackageReadWriteArchive>(accessor_.get())) {

		auto& packArch = static_cast<ref<PackageReadWriteArchive>>(accessor_.get());
		if (packArch.getLinked().data.size <= streamsize {}) {
			// nextLinked.changes;
			packArch.getLinked().changes.emplace_back(
				package::ArchiveDeltaAdd {
					.where = streamoff {},
					.sizedData = packArch
				}
			);

		} else {
			// maybeLinked.changes;
			packArch.getLinked().changes.emplace_back(
				package::ArchiveDeltaReplace {
					.where = streamoff {},
					.byteCount = packArch.getLinked().data.size,
					.withSizedData = packArch
				}
			);
		}

	}

	/**/

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

	const auto archiveGuid = storage_->getArchiveGuid();
	::hg::assertd(archiveGuid != ArchiveGuid {});

	auto& linker = accessor_->getLinker();
	auto linked = linker.get(archiveGuid);

	/**/

	if (linked == None) {
		return Unexpected<Query::acq_error_type> { AccessError { "Requested archive does not exist within package." } };
	}

	/**/

	constexpr auto contentOffset = sizeof(PackageHeader);
	auto value = Rc<StorageReadonlyArchive>::create(
		linker.getStorage(),
		static_cast<streamoff>(contentOffset + linked->data.offset),
		linked->data.size
	);
	value->setType(linked->header.type);

	auto cachedValue = ctx_.objectStore.add(
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
	auto storedResource = ctx_.accessStore.add(clone(storage_).into<IStorage>(), ::hg::move(resource));

	/**/

	return Expected(storedResource->acquireReadonly());
}

void engine::storage::releaseStaticPackArchQuery(
	ref<Context> ctx_,
	cref<Arci<system::ArchiveStorage>> storage_,
	mref<IoResourceAccessor<resource::StorageReadonlyArchive>> accessor_
) {
	auto deferred = ctx_.accessStore.drop(clone(storage_).into<IStorage>());
	ctx_.objectStore.invalidate(clone(storage_).into<IStorage>());

	::hg::forward<decltype(accessor_)>(accessor_).reset();
	::hg::discard(::hg::move(deferred));
}
