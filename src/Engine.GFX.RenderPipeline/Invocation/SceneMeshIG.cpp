#include "SceneMeshIG.hpp"

#include <Engine.Reflect/IsType.hpp>
#include <Engine.Scene/RenderGraph.hpp>

#include "SceneWalker.hpp"

using namespace hg::engine::gfx::render::pipeline;
using namespace hg;

SceneMeshIG::SceneMeshIG() noexcept = default;

SceneMeshIG::SceneMeshIG(mref<smr<SceneWalker>> sceneWalker_) noexcept :
    SceneMeshBaseIG(),
    _sceneWalker(_STD move(sceneWalker_)) {
    _sceneWalker->hook(*this);
}

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

void SceneMeshIG::operator()() {

    const scene::RenderGraph* sg {};
    const auto& sw = *_sceneWalker;
    sg->traversal(sw);

    /**/

    for (const auto& dynamicGenerator : getDynamicIgs()) {
        auto result = dynamicGenerator->finalize();
    }

    forEachStaticIg(
        [](auto& staticGenerator_) {
            auto result = staticGenerator_->finalize();
        }
    );
}

void SceneMeshIG::operator()(const ptr<const SceneWalker::scene_model_type> model_) {

    #ifdef _DEBUG
    assert(IsType<GeometryModel>(*model_));
    #endif

    uptr<IGCommandBuffer> buffer {};

    const auto model = static_cast<const ptr<const GeometryModel>>(model_);
    const auto modelTypeId = model_->getMetaClass()->typeId();

    model->render(buffer.get());

    auto& generator = selectSubGenerator(modelTypeId);
    auto result = generator(_STD move(buffer));
}

IGProcessResult SceneMeshIG::operator()(
    mref<smr<const acc::AccelerationPass>> acceleration_,
    mref<uptr<IGCommandBuffer>> igcb_
) {
    return IGProcessResult::eFailed;
}

tl::expected<engine::gfx::AccelerationCommandBuffer, IGError> SceneMeshIG::finalize() noexcept {
    return tl::expected<engine::gfx::AccelerationCommandBuffer, IGError> { tl::unexpect, IGError {} };
}
