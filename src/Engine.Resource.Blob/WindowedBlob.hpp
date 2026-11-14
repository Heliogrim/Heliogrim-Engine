#pragma once

#include <span>
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Reflect/Inherit/InheritMeta.hpp>

#include "Blob.hpp"

namespace hg::engine::resource {
	/**
	 * Read-only view of `[base, base + size)` of a source blob, served from one window-sized buffer.
	 * Not thread-safe for concurrent readers of one instance; use one instance per accessor.
	 */
	class WindowedBlob final :
		public InheritMeta<WindowedBlob, Blob> {
	public:
		constexpr static size_t kArchiveWindow = 8uz * 1024uz * 1024uz;

	public:
		WindowedBlob(ref<const Blob> source_, const streamoff base_, const streamsize size_, const size_t window_ = kArchiveWindow);

		~WindowedBlob() override = default;

	private:
		ref<const Blob> _source;
		streamoff _base;
		streamsize _size;
		size_t _window;

		mutable Vector<_::byte> _buffer;
		mutable streamoff _loaded;
		mutable size_t _loadedLen;

	private:
		void load(const size_t index_) const;

	public:
		std::span<_::byte> read(streamoff offset_, mref<std::span<_::byte>> dst_) const override;

		std::span<_::byte> write(streamoff offset_, mref<std::span<_::byte>> src_) override;

		bool trim(size_t tailSize_) override;

	public:
		[[nodiscard]] streamsize size() const noexcept override;
	};
}
