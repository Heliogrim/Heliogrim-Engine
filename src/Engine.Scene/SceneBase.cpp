#include "SceneBase.hpp"

#include "SceneSystem.hpp"

using namespace hg::engine::scene;
using namespace hg;

SceneBase::SceneBase() noexcept = default;

SceneBase::~SceneBase() noexcept = default;

void SceneBase::forEachSystem(FnRef<void(ref<SceneSystemBase>)> fn_) const {
	for (size_t index = 0uLL; const auto system = getSceneSystem(index); ++index) {
		fn_(*system.get());
	}
}
