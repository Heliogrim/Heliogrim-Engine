#pragma once

#include "../Blob.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
import Engine.Reflect;
#endif

namespace hg::engine::resource {
	/**
	 * Growable in-memory blob; a write at `offset_ <= size()` extends it. Backs the scratch image of a replace.
	 */
	class HeapBlob final :
		public InheritMeta<HeapBlob, Blob> {
	public:
		explicit HeapBlob(const size_t reserve_);

		~HeapBlob() override = default;

	private:
		Vector<_::byte> _bytes;

	public:
		std::span<_::byte> read(streamoff offset_, mref<std::span<_::byte>> dst_) const override;

		std::span<_::byte> write(streamoff offset_, mref<std::span<_::byte>> src_) override;

		bool trim(size_t tailSize_) override;

	public:
		[[nodiscard]] std::span<const _::byte> bytes() const noexcept;

		[[nodiscard]] streamsize size() const noexcept override;
	};
}
