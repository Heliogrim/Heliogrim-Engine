#pragma once

#include <Engine.Resource/FileTypeId.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import std;
#endif

namespace hg::engine::graphics {
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
