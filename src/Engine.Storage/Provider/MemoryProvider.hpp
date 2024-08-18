#pragma once
#include <memory_resource>
#include <Engine.Common/Collection/InlineAutoArray.hpp>
#include <Engine.Common/Managed/Rc.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>

#include "../IStorageProvider.hpp"

namespace hg::engine::storage {
	class IStorageRegistry;
	class MemoryStorageDescriptor;
}

namespace hg::engine::storage::system {
	class MemoryStorage;
	class MemoryRepository;
}

namespace hg::engine::storage::system {
	class MemoryProvider final :
		public IStorageProvider {
	public:
		friend class ::hg::engine::storage::system::MemoryRepository;

	public:
		using this_type = MemoryProvider;

	public:
		explicit MemoryProvider(mref<uptr<std::pmr::memory_resource>> allocator_) noexcept;

		~MemoryProvider() override;

	private:
		uptr<std::pmr::memory_resource> _allocator;
		InlineAutoArray<uptr<MemoryRepository>> _repositories;

	protected:
		[[nodiscard]] Arci<MemoryStorage> makeStorageObject(mref<MemoryStorageDescriptor> descriptor_);

	public:
		[[nodiscard]] nmpt<MemoryRepository> makeMemoryRepository();
	};
}
