#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.GFX/Scene/GeometryModel.hpp>
#include <Engine.GFX/Scene/SkeletalGeometryModel.hpp>
#include <Engine.GFX/Scene/StaticGeometryModel.hpp>

#include "InvocationGenerator.hpp"
#include "../__fwd.hpp"

namespace hg::engine::gfx::render::pipeline {
    class SceneMeshIG :
        public InvocationGenerator {
    public:
        smr<SceneWalker> _sceneWalker;

    private:
        [[nodiscard]] nmpt<IGCommandBuffer> getIgCmdBuffer();

    public:
        void accept(mref<nmpt<const GeometryModel>> model_) {

            if (model_->getClass()->isExactType<StaticGeometryModel>()) {
                // nmpt<const StaticGeometryModel> model = _STD move(model_);

            } else if (model_->getClass()->isExactType<SkeletalGeometryModel>()) {
                // nmpt<const SkeletalGeometryModel> model = _STD move(model_);
            }

            // TODO:
            model_->render(getIgCmdBuffer());
        }
    };
}
