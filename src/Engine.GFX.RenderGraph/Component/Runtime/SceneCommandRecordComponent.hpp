#pragma once

#include <Engine.GFX.Render.Graph.Scene/SceneWalker.hpp>

#include "CommandRecordComponent.hpp"

namespace hg::engine::gfx::render::graph {
    class SceneCommandRecordComponent final :
        public InheritMeta<SceneCommandRecordComponent, CommandRecordComponent> {
    public:
        using this_type = SceneCommandRecordComponent;

    public:
        SceneCommandRecordComponent() noexcept = delete;

        SceneCommandRecordComponent(mref<SceneWalker> sceneWalker_) noexcept;

        constexpr ~SceneCommandRecordComponent() noexcept override = default;

    private:
        SceneWalker _sceneWalker;

    public:
        [[nodiscard]] RecordInvalidationResult iterate(cref<IterationPassContext> ctx_) override;

    public:
        [[nodiscard]] cref<cmd_view_enum> getEnumeration() const noexcept override;
    };
}
