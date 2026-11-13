#pragma once

#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.Core/Module/RootModule.hpp>
#include <Engine.IO.Dispatcher/IoSystem.hpp>

namespace hg::engine {
	class Config;
	class Engine;
}

namespace hg::engine::storage {
	class IStorageRegistry;
	class StorageActionResolver;
	class StorageSystem;
}

namespace hg::engine {
	class StorageModule final :
		public core::RootModule {
	public:
		using this_type = StorageModule;

		using IStorageRegistry = ::hg::engine::storage::IStorageRegistry;
		using StorageActionResolver = ::hg::engine::storage::StorageActionResolver;
		using System = ::hg::engine::storage::StorageSystem;

	public:
		explicit StorageModule(ref<Engine> engine_);

		StorageModule(cref<this_type>) = delete;

		StorageModule(mref<this_type>) = delete;

		~StorageModule();

	public:
		void setup(_In_ cref<Config> config_);

		// Stops the io thread, then drops cached stages; later file access sees a stopped io and is synchronous.
		void stopIo();

	private:
		// Note: Declared first, so cached files and queues of the system die before it.
		io::IoSystem _io;
		uptr<IStorageRegistry> _registry;
		uptr<StorageActionResolver> _resolver;
		uptr<System> _system;

	public:
		[[nodiscard]] nmpt<IStorageRegistry> getRegistry() const noexcept;

		[[nodiscard]] nmpt<System> getSystem() const noexcept;

		[[nodiscard]] ref<io::IoSystem> io() noexcept;
	};
}
