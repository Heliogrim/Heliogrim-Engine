#pragma once

#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.Core/Module/RootModule.hpp>

namespace hg::engine {
	class Config;
	class Engine;
}

namespace hg::engine::storage {
	class IStorageRegistry;
	class StorageIo;
}

namespace hg::engine {
	class StorageModule final :
		public core::RootModule {
	public:
		using this_type = StorageModule;

		using IStorageRegistry = ::hg::engine::storage::IStorageRegistry;
		using StorageIo = ::hg::engine::storage::StorageIo;

	public:
		explicit StorageModule(ref<Engine> engine_);

		StorageModule(cref<this_type>) = delete;

		StorageModule(mref<this_type>) = delete;

		~StorageModule();

	public:
		void setup(_In_ cref<Config> config_);

	private:
		uptr<IStorageRegistry> _registry;
		uptr<StorageIo> _io;

	public:
		[[nodiscard]] nmpt<IStorageRegistry> getRegistry() const noexcept;

		[[nodiscard]] nmpt<StorageIo> getIo() const noexcept;
	};
}
