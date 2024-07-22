#include "StorageIo.hpp"

#include <Engine.Asserts/Asserts.hpp>
#include <Engine.Asserts/Todo.hpp>

#include "Engine.Pedantic.Resource/ExclusiveResource.hpp"
#include "Engine.Reflect/IsType.hpp"
#include "Engine.Reflect/TypeSwitch.hpp"
#include "Engine.Resource.Blob/File/LfsSyncBlob.hpp"
#include "Engine.Resource.Blob/Memory/ByteSpanBlob.hpp"
#include "Engine.Resource.Package/External/PackageIo.hpp"
#include "Engine.Storage/IStorage.hpp"
#include "Engine.Storage/Storage/LocalFileStorage.hpp"
#include "Engine.Storage/Storage/MemoryStorage.hpp"
#include "Engine.Storage/Storage/PackageStorage.hpp"

using namespace hg::engine::storage;
using namespace hg;

StorageIo::StorageIo(ref<Engine> engine_) noexcept :
	_engine(std::addressof(engine_)) {}

UniqueResourceAccessor<Io<engine::resource::MemoryBuffer>> StorageIo::accessReadWrite(
	mref<Arci<system::MemoryStorage>> storage_
) const {

	using io_type = Io<::hg::engine::resource::MemoryBuffer>;
	using resource_type = decltype(make_exclusive_resource<io_type>());

	// Attention: Should be unique, but std::function for unique_ptr<T, Delete<T>> requires deleter to be copyable
	auto resource = make_smr<resource_type>(io_type { std::addressof(storage_->_memory) });

	return UniqueResourceAccessor<Io<engine::resource::MemoryBuffer>> {
		resource->acquireReadWrite(),
		[resourceKeepAlive = std::move(resource), storage = std::move(storage_)](auto* accessor_) mutable -> void {
			::hg::assertrt(accessor_ != nullptr && resourceKeepAlive != nullptr);
			static_cast<ptr<decltype(resource->acquireReadWrite())>>(accessor_)->reset();
			resourceKeepAlive.reset();
			storage.reset();
		}
	};
}

UniqueResourceAccessor<const Io<const engine::resource::MemoryBuffer>> StorageIo::accessReadonly(
	mref<Arci<system::MemoryStorage>> storage_
) const {

	using io_type = const Io<const ::hg::engine::resource::MemoryBuffer>;
	using resource_type = decltype(make_exclusive_resource<io_type>({ nullptr }));

	// Attention: Should be unique, but std::function for unique_ptr<T, Delete<T>> requires deleter to be copyable
	auto resource = make_smr<resource_type>(io_type { std::addressof(storage_->_memory) });

	return UniqueResourceAccessor<io_type> {
		resource->acquireReadonly(),
		[resourceKeepAlive = std::move(resource), storage = std::move(storage_)](auto* accessor_) mutable -> void {
			::hg::assertrt(accessor_ != nullptr && resourceKeepAlive != nullptr);
			static_cast<ptr<decltype(resourceKeepAlive->acquireReadonly())>>(accessor_)->reset();
			resourceKeepAlive.reset();
			storage.reset();
		}
	};
}

AccessPackageReadonly StorageIo::accessReadonly(
	mref<Arci<system::PackageStorage>> storage_
) const {

	auto backing = storage_->_backing;
	auto blobAccess = accessReadonlyBlob(std::move(backing));
	auto sharedBlobAccess = make_smr(std::move(blobAccess));

	// Problem: This will only work as long as PackageIo does not have any state.
	::hg::assertrt(sizeof(PackageIo) == sizeof(nmpt<StorageIo>));

	auto pio = PackageIo { const_cast<ref<StorageIo>>(*this) };
	// Warning: We just assume that we have already initialized memory; We need to handle this explicit
	auto package = pio.create_package_from_storage(*sharedBlobAccess);
	auto sharedPackage = make_smr(std::move(package));

	/**/

	using io_type = Io<const ::hg::engine::resource::Package>;
	auto sharedResource = make_smr(make_exclusive_resource(io_type { sharedPackage.get() }));

	return AccessPackageReadonly {
		sharedResource->acquireReadonly(),
		[
			resourceKeepAlive = std::move(sharedResource),
			packageKeepAlive = std::move(sharedPackage),
			// Note: Actually obsolete, as the active accessor will lock the storage object
			blob = std::move(sharedBlobAccess),
			storage = std::move(storage_)
		](auto* accessor_) mutable -> void {
			// Warning: Temporary indirect "release" of the buffer used within the deprecated buffer source
			::hg::assertrt(packageKeepAlive != nullptr);

			packageKeepAlive.reset();

			::hg::assertrt(accessor_ != nullptr && resourceKeepAlive != nullptr && storage != nullptr);
			static_cast<ptr<decltype(resourceKeepAlive->acquireReadonly())>>(accessor_)->reset();
			resourceKeepAlive.reset();
			blob.reset();
			storage.reset();
		}
	};
}

AccessPackageReadWrite StorageIo::accessReadWrite(
	mref<Arci<system::PackageStorage>> storage_
) const {

	auto blobAccess = accessReadWriteBlob(clone(storage_->_backing));
	auto sharedBlobAccess = make_smr(std::move(blobAccess));

	// Problem: This will only work as long as PackageIo does not have any state.
	::hg::assertrt(sizeof(PackageIo) == sizeof(nmpt<StorageIo>));

	auto pio = PackageIo { const_cast<ref<StorageIo>>(*this) };
	// Warning: We just assume that we have uninitialized memory; We need to handle this explicit
	auto package = pio.create_empty_package(*sharedBlobAccess);
	auto sharedPackage = make_smr(std::move(package));

	/**/

	using io_type = Io<::hg::engine::resource::Package>;
	auto sharedResource = make_smr(make_exclusive_resource(io_type { sharedPackage.get() }));

	return AccessPackageReadWrite {
		sharedResource->acquireReadWrite(),
		[
			resourceKeepAlive = std::move(sharedResource),
			packageKeepAlive = std::move(sharedPackage),
			// Note: Actually obsolete, as the active accessor will lock the storage object
			blob = std::move(sharedBlobAccess),
			storage = std::move(storage_)
		](auto* accessor_) mutable -> void {
			// Warning: Temporary indirect "release" of the buffer used within the deprecated buffer source
			::hg::assertrt(packageKeepAlive != nullptr);

			packageKeepAlive.reset();

			::hg::assertrt(accessor_ != nullptr && resourceKeepAlive != nullptr && storage != nullptr);
			static_cast<ptr<decltype(resourceKeepAlive->acquireReadWrite())>>(accessor_)->reset();
			resourceKeepAlive.reset();
			blob.reset();
			storage.reset();
		}
	};
}

#pragma region Storage Io Blob Handling

AccessBlobReadonly StorageIo::accessReadonlyBlob(
	mref<Arci<system::MemoryStorage>> storage_
) const {

	auto sharedBlob = make_smr<resource::ByteSpanBlob>(storage_->_memory.span());
	auto sharedResource = make_smr(make_exclusive_resource(Io<const resource::Blob> { sharedBlob.get() }));

	return AccessBlobReadonly {
		sharedResource->acquireReadonly(),
		[
			resource = std::move(sharedResource),
			blob = std::move(sharedBlob),
			// Note: Actually obsolete, as the active accessor will lock the storage object
			storage = std::move(storage_)
		](auto* accessor_) mutable -> void {
			// Note: Validate the current resource accessor and release locking reference
			::hg::assertrt(accessor_ != nullptr && resource != nullptr && storage != nullptr);
			static_cast<ptr<decltype(resource->acquireReadonly())>>(accessor_)->reset();
			resource.reset();

			// Note: Release the internally allocates resource
			::hg::assertrt(blob != nullptr);
			blob.reset();

			// Note: Release (obsolete) storage locking reference
			storage.reset();
		}
	};
}

AccessBlobReadonly StorageIo::accessReadonlyBlob(mref<Arci<system::LocalFileStorage>> storage_) const {

	auto sharedBlob = make_smr<resource::LfsSyncBlob>(clone(storage_->_lfsPath));
	auto sharedResource = make_smr(make_exclusive_resource(Io<const resource::Blob> { sharedBlob.get() }));

	return AccessBlobReadonly {
		sharedResource->acquireReadonly(),
		[
			resource = std::move(sharedResource),
			blob = std::move(sharedBlob),
			// Note: Actually obsolete, as the active accessor will lock the storage object
			storage = std::move(storage_)
		](auto* accessor_) mutable -> void {
			// Note: Validate the current resource accessor and release locking reference
			::hg::assertrt(accessor_ != nullptr && resource != nullptr && storage != nullptr);
			static_cast<ptr<decltype(resource->acquireReadonly())>>(accessor_)->reset();
			resource.reset();

			// Note: Release the internally allocates resource
			::hg::assertrt(blob != nullptr);
			blob.reset();

			// Note: Release (obsolete) storage locking reference
			storage.reset();
		}
	};
}

AccessBlobReadonly StorageIo::accessReadonlyBlob(
	mref<Arci<IStorage>> storage_
) const {
	::hg::assertrt(IsType<system::MemoryStorage>(*storage_) || IsType<system::LocalFileStorage>(*storage_));
	return switchType(
		storage_.get(),
		[this](const ptr<system::MemoryStorage> typed_) {
			return accessReadonlyBlob(typed_->arci_from_this<>().into<system::MemoryStorage>());
		},
		[this](const ptr<system::LocalFileStorage> typed_) {
			return accessReadonlyBlob(typed_->arci_from_this<>().into<system::LocalFileStorage>());
		}
	);
}

AccessBlobReadWrite StorageIo::accessReadWriteBlob(
	mref<Arci<system::MemoryStorage>> storage_
) const {

	auto sharedBlob = make_smr<resource::ByteSpanBlob>(storage_->_memory.span());
	auto sharedResource = make_smr(make_exclusive_resource(Io<resource::Blob> { sharedBlob.get() }));

	return AccessBlobReadWrite {
		sharedResource->acquireReadWrite(),
		[
			resource = std::move(sharedResource),
			blob = std::move(sharedBlob),
			// Note: Actually obsolete, as the active accessor will lock the storage object
			storage = std::move(storage_)
		](auto* accessor_) mutable -> void {
			// Note: Validate the current resource accessor and release locking reference
			::hg::assertrt(accessor_ != nullptr && resource != nullptr && storage != nullptr);
			static_cast<ptr<decltype(resource->acquireReadWrite())>>(accessor_)->reset();
			resource.reset();

			// Note: Release the internally allocates resource
			::hg::assertrt(blob != nullptr);
			blob.reset();

			// Note: Release (obsolete) storage locking reference
			storage.reset();
		}
	};
}

AccessBlobReadWrite StorageIo::accessReadWriteBlob(mref<Arci<system::LocalFileStorage>> storage_) const {

	auto sharedBlob = make_smr<resource::LfsSyncBlob>(clone(storage_->_lfsPath));
	auto sharedResource = make_smr(make_exclusive_resource(Io<resource::Blob> { sharedBlob.get() }));

	return AccessBlobReadWrite {
		sharedResource->acquireReadWrite(),
		[
			resource = std::move(sharedResource),
			blob = std::move(sharedBlob),
			// Note: Actually obsolete, as the active accessor will lock the storage object
			storage = std::move(storage_)
		](auto* accessor_) mutable -> void {
			// Note: Validate the current resource accessor and release locking reference
			::hg::assertrt(accessor_ != nullptr && resource != nullptr && storage != nullptr);
			static_cast<ptr<decltype(resource->acquireReadWrite())>>(accessor_)->reset();
			resource.reset();

			// Note: Release the internally allocates resource
			::hg::assertrt(blob != nullptr);
			blob.reset();

			// Note: Release (obsolete) storage locking reference
			storage.reset();
		}
	};
}

AccessBlobReadWrite StorageIo::accessReadWriteBlob(
	mref<Arci<IStorage>> storage_
) const {
	::hg::assertrt(IsType<system::MemoryStorage>(*storage_) || IsType<system::LocalFileStorage>(*storage_));
	return switchType(
		storage_.get(),
		[this](const ptr<system::MemoryStorage> typed_) {
			return accessReadWriteBlob(typed_->arci_from_this<>().into<system::MemoryStorage>());
		},
		[this](const ptr<system::LocalFileStorage> typed_) {
			return accessReadWriteBlob(typed_->arci_from_this<>().into<system::LocalFileStorage>());
		}
	);
}

#pragma endregion Storage Io Blob Handling
