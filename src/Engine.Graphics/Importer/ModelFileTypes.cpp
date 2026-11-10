#include "ModelFileTypes.hpp"

#include <array>

using namespace hg::engine::gfx;
using namespace hg;

const engine::res::FileTypeId ModelFileType::Fbx { "fbx"_typeId, ".fbx" };
const engine::res::FileTypeId ModelFileType::Obj { "obj"_typeId, ".obj" };

static auto modelFileTypes = std::array {
	ModelFileType::Fbx,
	ModelFileType::Obj
};

std::span<const engine::res::FileTypeId> ModelFileType::asSpan() noexcept {
	return modelFileTypes;
}
