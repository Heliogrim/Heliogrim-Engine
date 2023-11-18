#pragma once
#include <Engine.GFX.Render.Graph.Scene/SceneWalker.hpp>
#include <Engine.GFX.Render.Graph.Scene/Hook/ModelType.hpp>
#include <Engine.Scene/IRenderScene.hpp>

#include "Engine.GFX.Render.Subpass/Mesh/MeshSubPass.hpp"

namespace hg::engine::gfx::render {
    class DepthPrePass :
        public MeshSubPass {
    public:
        void iterate() noexcept override {

            scene::SceneView sceneView {};
            graph::SceneWalker walker {};

            Vector<ptr<const MetaClass>> typeList = { GeometryModel::getStaticMetaClass() };
            walker.addHookFilter(make_uptr<graph::ModelTypeFilter>(_STD move(typeList)));
            walker.setHook(
                [ci = createCaptureObject()](const auto* const model_) {
                    static_cast<const ptr<const GeometryModel>>(model_)->capture(ci);
                }
            );

            walker(sceneView);

        }

        void resolve() noexcept override {}

        void execute() noexcept override {}
    };
}
