#include "FontFileTypes.hpp"

#include <array>

using namespace hg::engine::gfx;
using namespace hg;

const engine::res::FileTypeId FontFileType::Ttf { "ttf"_typeId, ".ttf" };
const engine::res::FileTypeId FontFileType::Otf { "otf"_typeId, ".otf" };

static auto fontFileTypes = std::array {
	FontFileType::Ttf,
	FontFileType::Otf
};

std::span<const engine::res::FileTypeId> FontFileType::asSpan() noexcept {
	return fontFileTypes;
}
