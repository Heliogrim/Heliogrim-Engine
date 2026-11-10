#include "ImageFileTypes.hpp"

#include <array>

using namespace hg::engine::gfx;
using namespace hg;

const engine::res::FileTypeId ImageFileType::Exr { "exr"_typeId, ".exr" };
const engine::res::FileTypeId ImageFileType::Hdr { "hdr"_typeId, ".hdr" };
const engine::res::FileTypeId ImageFileType::Jpeg { "jpeg"_typeId, ".jpg" };
const engine::res::FileTypeId ImageFileType::Ktx { "ktx"_typeId, ".ktx" };
const engine::res::FileTypeId ImageFileType::Ktx2 { "ktx2"_typeId, ".ktx2" };
const engine::res::FileTypeId ImageFileType::Png { "png"_typeId, ".png" };

static auto imageFileTypes = std::array {
	ImageFileType::Exr,
	ImageFileType::Hdr,
	ImageFileType::Jpeg,
	ImageFileType::Ktx,
	ImageFileType::Ktx2,
	ImageFileType::Png,
};

std::span<const engine::res::FileTypeId> ImageFileType::asSpan() noexcept {
	return imageFileTypes;
}
