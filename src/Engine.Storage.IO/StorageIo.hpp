#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>

namespace hg::engine {
	class Engine;
}

namespace hg::engine::storage {
	class IStorage;
	class StorageInputStream;
	class StorageOutputStream;
}

namespace hg::engine::storage {
	class StorageIo final {
	public:
		using this_type = StorageIo;

	public:
		explicit StorageIo(_In_ ref<Engine> engine_) noexcept;

		StorageIo(cref<this_type>) = delete;

		StorageIo(mref<this_type>) = delete;

		constexpr ~StorageIo() noexcept = default;

	private:
		nmpt<Engine> _engine;

	public:
		[[nodiscard]] uptr<StorageInputStream> makeInputStream(_In_ mref<IStorage> storage_);

		[[nodiscard]] uptr<StorageOutputStream> makeOutputStream(_In_ mref<IStorage> storage_);
	};
}
