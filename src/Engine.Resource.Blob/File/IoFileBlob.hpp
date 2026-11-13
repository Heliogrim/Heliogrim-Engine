#pragma once

#include <span>
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Managed/Rc.hpp>
#include <Engine.IO.Dispatcher/IoSystem.hpp>
#include <Engine.Reflect/Inherit/InheritMeta.hpp>

#include "../Blob.hpp"

namespace hg::engine::resource {
	/**
	 * Blob over an overlapped file; every call submits one request and awaits it.
	 */
	class IoFileBlob final :
		public InheritMeta<IoFileBlob, Blob> {
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
		[[nodiscard]] std::span<_::byte> transfer(const io::IoOp op_, const streamoff offset_, const std::span<_::byte> buffer_) const;

	public:
		std::span<_::byte> read(streamoff offset_, mref<std::span<_::byte>> dst_) const override;

		std::span<_::byte> write(streamoff offset_, mref<std::span<_::byte>> src_) override;

		bool trim(size_t tailSize_) override;

	public:
		[[nodiscard]] streamsize size() const noexcept override;
	};
}
