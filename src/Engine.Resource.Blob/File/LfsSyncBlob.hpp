#pragma once

#include <Engine.Async/Primitive/FiberMutex.hpp>
#include <Engine.Filesystem/Url.hpp>

#include "../Blob.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
import Engine.Reflect;
#endif

namespace hg::engine::resource {
	// TODO: Determine how we should handle the difference between 'readWrite' and 'readonly' access.
	class LfsSyncBlob final :
		public InheritMeta<LfsSyncBlob, Blob> {
	public:
		explicit LfsSyncBlob(mref<fs::Path> path_) noexcept;

		LfsSyncBlob(cref<LfsSyncBlob>) = delete;

		LfsSyncBlob(mref<LfsSyncBlob>) = delete;

		constexpr ~LfsSyncBlob() override = default;

	private:
		// Note: We need this for resizing operations on the target file...
		fs::Path _path;
		UniqueValue<std::fstream> _stream;
		// Note: Set when the file only opens for reading; write and trim assert against it.
		bool _readonly = false;
		// shortcut: one stream per blob, W03.3 replaces it with positional overlapped reads
		mutable concurrent::FiberMutex _streamLock;

	public:
		std::span<_::byte> read(streamoff offset_, mref<std::span<_::byte>> dst_) const override;

		std::span<_::byte> write(streamoff offset_, mref<std::span<_::byte>> src_) override;

		bool trim(size_t tailSize_) override;

	public:
		[[nodiscard]] streamsize size() const noexcept override;
	};
}
