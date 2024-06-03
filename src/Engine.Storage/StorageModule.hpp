#pragma once

#include <Engine.Common/Wrapper.hpp>

#include <Engine.Core/Module/CoreModule.hpp>

namespace hg::engine::storage {
	class IStorageRegistry;
	class StorageIo;
}

namespace hg::engine {
	class StorageModule final :
		public core::CoreModule {
	public:
		using this_type = StorageModule;

		using IStorageRegistry = ::hg::engine::storage::IStorageRegistry;
		using StorageIo = ::hg::engine::storage::StorageIo;

	public:
		explicit StorageModule(ref<Engine> engine_);

		StorageModule(cref<this_type>) = delete;

		StorageModule(mref<this_type>) = delete;

		~StorageModule() override;

	public:
		void setup() override;

		void start() override;

		void stop() override;

		void destroy() override;

	private:
		uptr<IStorageRegistry> _registry;
		uptr<StorageIo> _io;

	public:
		[[nodiscard]] nmpt<IStorageRegistry> getRegistry() const noexcept;

		[[nodiscard]] nmpt<StorageIo> getIo() const noexcept;
	};
}
