#pragma once

#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Resource.Archive/StorageReadonlyArchive.hpp>
#include <Engine.Resource.Blob/WindowedBlob.hpp>

namespace hg::engine::storage {
	// True if `[base_, base_ + size_)` lies within `[0, sourceSize_)`; never overflows.
	[[nodiscard]] constexpr bool windowFits(const streamsize sourceSize_, const streamoff base_, const streamsize size_) noexcept {
		return base_ >= 0LL && size_ >= 0LL && base_ <= sourceSize_ && size_ <= sourceSize_ - base_;
	}

	/**
	 * Readonly archive over `[base, base + size)` of a blob, read through one window per query.
	 */
	struct WindowedArchive {
		WindowedArchive(_In_ ref<const resource::Blob> source_, _In_ const streamoff base_, _In_ const streamsize size_) :
			window(source_, base_, size_),
			archive(window, streamoff {}, size_) {}

		WindowedArchive(ref<const WindowedArchive>) = delete;

		WindowedArchive(mref<WindowedArchive>) = delete;

		resource::WindowedBlob window;
		resource::StorageReadonlyArchive archive;
	};
}
