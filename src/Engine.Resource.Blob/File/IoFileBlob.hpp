#pragma once

#include "../Blob.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
import Engine.IO.Dispatcher;
import Engine.Reflect;
#endif

namespace hg::engine::resource {
	/**
	 * Blob over an overlapped file; every call submits one batch and awaits it.
	 * Reads split into requests of at most `max_read_request` bytes; on an unbuffered file a misaligned read goes
	 * through one aligned bounce buffer over the rounded range.
	 * If any piece fails, the read returns no bytes; a short piece ends the read after the bytes before it.
	 */
	class IoFileBlob final :
		public InheritMeta<IoFileBlob, Blob> {
	public:
		// Q03.9: about 256 KiB per request reaches the drive ceiling at a shallow queue.
		constexpr static size_t max_read_request = 256uz * 1024uz;

	public:
		IoFileBlob(ref<io::IoSystem> system_, Arci<io::IoFile> file_, Arci<io::IoQueue> queue_);

		IoFileBlob(ref<const IoFileBlob>) = delete;

		IoFileBlob(mref<IoFileBlob>) = delete;

		~IoFileBlob() override = default;

	private:
		ref<io::IoSystem> _system;
		Arci<io::IoFile> _file;
		Arci<io::IoQueue> _queue;
		streamsize _size;

	private:
		// Bytes done from the start of buffer_, up to the first piece that did not complete in full.
		[[nodiscard]] size_t transfer(const io::IoOp op_, const streamoff offset_, const std::span<_::byte> buffer_, const size_t piece_) const;

	public:
		std::span<_::byte> read(streamoff offset_, mref<std::span<_::byte>> dst_) const override;

		std::span<_::byte> write(streamoff offset_, mref<std::span<_::byte>> src_) override;

		bool trim(size_t tailSize_) override;

	public:
		[[nodiscard]] streamsize size() const noexcept override;
	};
}
