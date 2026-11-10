#pragma once

#include <span>
#include <Engine.Resource/FileTypeId.hpp>

namespace hg::engine::gfx {
	namespace ImageFileType {
		extern const res::FileTypeId Exr;// TODO: Check for support
		extern const res::FileTypeId Hdr;// TODO: Check for support
		extern const res::FileTypeId Jpeg;
		extern const res::FileTypeId Ktx;
		extern const res::FileTypeId Ktx2;
		extern const res::FileTypeId Png;

		[[nodiscard]] extern std::span<const res::FileTypeId> asSpan() noexcept;
	}
}
