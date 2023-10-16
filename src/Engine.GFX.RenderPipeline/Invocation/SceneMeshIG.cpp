#include "SceneMeshIG.hpp"

using namespace hg::engine::gfx::render::pipeline;
using namespace hg;

SceneMeshIG::SceneMeshIG() = default;

SceneMeshIG::~SceneMeshIG() = default;

nmpt<IGCommandBuffer> SceneMeshIG::getIgCmdBuffer() {
    return {};
}

ref<StaticMeshIG> SceneMeshIG::getStaticMeshIg() noexcept {
    return getStaticIg<StaticMeshIG>();
}

ref<ProcMeshIG> SceneMeshIG::getProcMeshIg() noexcept {
    return getStaticIg<ProcMeshIG>();
}
