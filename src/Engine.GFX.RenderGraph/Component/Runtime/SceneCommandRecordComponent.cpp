#include "SceneCommandRecordComponent.hpp"

using namespace hg::engine::gfx::render::graph;
using namespace hg::engine::gfx;
using namespace hg;

SceneCommandRecordComponent::SceneCommandRecordComponent(mref<SceneWalker> sceneWalker_) noexcept :
    InheritMeta(),
    _sceneWalker(_STD move(sceneWalker_)) {}

RecordInvalidationResult SceneCommandRecordComponent::iterate(cref<IterationPassContext> ctx_) {

    // TODO: e.g Invoke Scene Walker

    cmd_view_enum localCmdView {};

    return RecordInvalidationResult::eAlways;
}

cref<CommandRecordComponent::cmd_view_enum> SceneCommandRecordComponent::getEnumeration() const noexcept {
    // Error // TODO
    cmd_view_enum localCmdView {};
    return localCmdView;
}
