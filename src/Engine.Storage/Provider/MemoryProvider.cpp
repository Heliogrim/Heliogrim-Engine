#include "MemoryProvider.hpp"

#include <Engine.Asserts/Asserts.hpp>
#include <Engine.Asserts/Todo.hpp>
#include <Engine.Common/Make.hpp>
#include <Engine.Common/Managed/AtomicRefCountedIntrusive.hpp>

#include "../Options/MemoryStorageDescriptor.hpp"
#include"../Repository/MemoryRepository.hpp"
#include "../Storage/MemoryStorage.hpp"

using namespace hg::engine::storage::system;
using namespace hg;

MemoryProvider::MemoryProvider(mref<uptr<std::pmr::memory_resource>> allocator_) noexcept :
	IStorageProvider(),
	_allocator(std::move(allocator_)) {}

MemoryProvider::~MemoryProvider() = default;

Arci<MemoryStorage> MemoryProvider::makeStorageObject(mref<MemoryStorageDescriptor> descriptor_) {
	return Arci<MemoryStorage>::create(
		MemoryStorage::MemoryObject {
			static_cast<ptr<_::byte>>(_allocator->allocate(descriptor_.memorySize())),
			descriptor_.memorySize(),
			[alloc = _allocator.get()](ptr<_::byte> mem_, size_t size_) {
				alloc->deallocate(mem_, size_);
			}
		},
		true,
		true,
		true,
		true
	);
}

nmpt<MemoryRepository> MemoryProvider::makeMemoryRepository() {
	::hg::assertrt(_repositories.empty());
	_repositories.emplace_back(new MemoryRepository(*this));
	return _repositories.front().get();
}
