#pragma once

#include "InvocationGenerator.hpp"

namespace hg::engine::gfx::render::pipeline {
    /**
     * Procedural mesh invocation generator is a specialization of a mesh invocation generator used to dispatch
     *  passes and materials bound to a procedurally generated geometry. Depending on the previous operations
     *  stated within the render graph used to generate the containing pipeline, this might used additional
     *  acceleration passes to generate the input data used for the actual mesh invocation. Alternatively it
     *  might consume a previously generated dynamic buffer, provided by an earlier stage from the pipeline.
     */
    class ProcMeshIG :
        public InvocationGenerator {
    public:
    };
}
