#include "SceneCommandRecordComponent.hpp"

#include <Engine.GFX/Scene/GeometryModel.hpp>
#include <Engine.GFX.Render.Command/RenderCommandPool.hpp>
#include <Engine.GFX.Render.Predefined/Symbols/SceneView.hpp>
#include <Engine.Reflect/Cast.hpp>

using namespace hg::engine::gfx::render::graph;
using namespace hg::engine::gfx;
using namespace hg;

SceneCommandRecordComponent::SceneCommandRecordComponent(mref<SceneWalker> sceneWalker_) noexcept :
    InheritMeta(),
    _sceneWalker(_STD move(sceneWalker_)) {}

RecordInvalidationResult SceneCommandRecordComponent::iterate(cref<IterationPassContext> ctx_) {

    /* Warning: Experimental */

    auto sceneViewSymbol = makeSceneViewSymbol();
    const auto imported = ctx_.symbols().importSymbol(_STD move(sceneViewSymbol));

    _sceneWalker.setHook(
        [this](const ptr<const SceneWalker::scene_model_type> model_) {

            const auto model = Cast<GeometryModel>(model_);
            if (not model) {
                return;
            }

            // Warning: Naive trivial implementation
            auto buffer = _renderCmdPool.acquire().value();
            model->render(buffer.get());

            _renderCmdViews.push_back(_renderCmdPool.view(buffer.get()).value());
            _renderCmdBuffers.push_back(_STD move(buffer));
        }
    );

    const auto* const view = static_cast<ptr<scene::SceneView>>(imported->data);
    _sceneWalker(*view);

    return RecordInvalidationResult::eAlways;
}

cref<CommandRecordComponent::cmd_view_enum> SceneCommandRecordComponent::getEnumeration() const noexcept {
    return _renderCmdViews;
}
