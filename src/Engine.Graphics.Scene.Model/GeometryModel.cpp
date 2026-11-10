#include "GeometryModel.hpp"

using namespace hg::engine::gfx::scene;
using namespace hg;

GeometryModel::GeometryModel(const ptr<SceneComponent> owner_) :
	InheritMeta(owner_) {}

Arc<engine::gfx::MaterialResource> GeometryModel::material(const u32 index_) const noexcept {
	if (index_ > _materials.size()) {
		return nullptr;
	}
	return _materials[index_];
}
