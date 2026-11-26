#pragma once

#include "../Blob.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Async;
import Engine.Common;
import Engine.Filesystem;
import Engine.Reflect;
#endif

namespace hg::engine::resource {
	// TODO: Determine how we should handle the difference between 'readWrite' and 'readonly' access.
	class LfsSyncBlob final :
		public InheritMeta<LfsSyncBlob, Blob> {
	public:
		explicit LfsSyncBlob(mref<fs::Path> path_) noexcept;

		// Adopts the handle of a temp sibling (createTempSibling); unless finished, the dtor removes path_.
		LfsSyncBlob(const ptr<void> tempHandle_, mref<fs::Path> tempPath_) noexcept;

		LfsSyncBlob(cref<LfsSyncBlob>) = delete;

		LfsSyncBlob(mref<LfsSyncBlob>) = delete;

		~LfsSyncBlob() override;

	private:
		// Note: We need this for resizing operations on the target file...
		fs::Path _path;
		// Note: Native handle sharing read, write and delete, so a commit can rename over the file while it is open.
		ptr<void> _handle = nullptr;
		// Note: Set when the file only opens for reading; write and trim assert against it.
		bool _readonly = false;
		bool _temp = false;
		bool _finished = false;
		mutable std::atomic<u32> _lastError = 0u;
		// shortcut: one lock per blob serialises positional calls, W03.3 replaces it with overlapped reads
		mutable concurrent::FiberMutex _streamLock;

	public:
		std::span<_::byte> read(streamoff offset_, mref<std::span<_::byte>> dst_) const override;

		std::span<_::byte> write(streamoff offset_, mref<std::span<_::byte>> src_) override;

		bool trim(size_t tailSize_) override;

	public:
		[[nodiscard]] Result<Rc<Blob>, io::CommitError> beginReplace(const streamsize size_) override;

		[[nodiscard]] Result<void, io::CommitError> finishReplace(mref<Rc<Blob>> replacement_) override;

		[[nodiscard]] u32 lastError() const noexcept override;

	public:
		[[nodiscard]] streamsize size() const noexcept override;
	};
}
