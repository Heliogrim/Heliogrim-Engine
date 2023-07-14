#pragma once

namespace hg::engine::gfx::render {
    class RenderPipeline;
}

namespace hg::engine::gfx::render::pipeline {
    class IGCommandBuffer;
    class InvocationGenerator;
    class SceneMeshIG;
    class SceneWalker;
    class SceneWalkerFilter;
    /**/
    class Stage;
    class SubpassStage;
    class AccelSubpassStage;
    class MultiAccelSubpassStage;
    class MaterialSubpassStage;
    /**/
    class State;
    class StateAware;
}
